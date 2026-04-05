unit UMainForm;


{$reference System.Drawing.dll}
{$reference System.Windows.Forms.dll}

{$resource res\icon.ico}
{$resource res\refresh.png}
{$resource res\connect.png}
{$resource res\disconnect.png}
{$resource res\paste.png}
{$resource res\open.png}


uses System;
uses System.IO;
uses System.IO.Ports;
uses System.Text.RegularExpressions;
uses System.Globalization;
uses System.Threading;
uses System.Threading.Tasks;
uses System.Diagnostics;
uses System.Drawing;
uses System.Windows.Forms;
uses UExtensions;
uses ULogBox;
uses URfidKey;


type
  TaskState = ( Running, Stoping, Completed );
  
  ResponseStatus =
  (
    Success               = $00,
    InvalidCommand        = $81,
    KeyQueueEmpty         = $82,
    InvalidLength         = $83,
    InvalidValue          = $84,
    InvalidPass           = $85,
    Timedout              = 1,
    ResponseInvalidLength = 2,
    ResponseInvalidCrc    = 3,
    ResponseUnknownCode   = -1
  );
  
  Response = class
    {$region Fields}
    private _Status: ResponseStatus;
    private _Data  : array of byte;
    {$endregion}
    
    {$region Ctors}
    public constructor (status: ResponseStatus; data: array of byte);
    begin
      _Status := status;
      _Data   := data;
    end;
    {$endregion}
    
    {$region Properties}
    public property Status: ResponseStatus read (_Status);
    
    public property Length: integer read (_Data <> nil ? _Data.Length : -1);
    
    public property Data: array of byte read (_Data);
    
    public property Bytes[i: integer]: byte read (_Data[i]); default;
    {$endregion}
  end;
  
  MainForm = class(Form)
    {$region Fields}
    private _PortBox        : System.Windows.Forms.GroupBox;
    private _PortSelect     : System.Windows.Forms.ComboBox;
    private _PortsUpdate    : System.Windows.Forms.Button;
    private _PortConnect    : System.Windows.Forms.Button;
    private _PortDisconnect : System.Windows.Forms.Button;
    private _ReadBox        : System.Windows.Forms.GroupBox;
    private _KeyReadStart   : System.Windows.Forms.Button;
    private _KeyReadStop    : System.Windows.Forms.Button;
    private _WriteBox       : System.Windows.Forms.GroupBox;
    private _TypeT55X7      : System.Windows.Forms.RadioButton;
    private _TypeEM4X05     : System.Windows.Forms.RadioButton;
    private _KeyWrite       : System.Windows.Forms.Button;
    private _KeySelect      : array of System.Windows.Forms.CheckBox;
    private _KeyValues      : array of System.Windows.Forms.TextBox;
    private _Log            : LogBox;
    private _UpgradeBox     : System.Windows.Forms.GroupBox;
    private _BootStart      : System.Windows.Forms.Button;
    private _FirmwareFname  : System.Windows.Forms.TextBox;
    private _FirmwareSelect : System.Windows.Forms.Button;
    private _FirmwareUpload : System.Windows.Forms.Button;
    private _Uart           : SerialPort;
    private _ReadTaskState  : TaskState;
    {$endregion}
    
    {$region Constants}
    private const ENCODE_BYTE_LENGTH   = 2;
    private const ENCODE_CRC_LENGTH    = 1;
    private const ENCODE_OFFSET        = $47;
    private const CMD_START_BOOTLOADER = $02;
    private const CMD_GET_COUNT_KEY    = $04;
    private const CMD_GET_NEXT_KEY     = $08;
    private const CMD_RESET_QUEUE      = $10;
    private const CMD_WRITE_T55X7      = $20;
    private const CMD_WRITE_EM4X05     = $40;
    private const CMD_WRITE_KEY_MASK   = $03;
    private const CMD_BOOTLOADER_PASS  = $5AA5;
    private const RESP_SUCCESS         = $00;
    private const RESP_INVALID_COMMAND = $81;
    private const RESP_KEY_QUEUE_EMPTY = $82;
    private const RESP_INVALID_LENGTH  = $83;
    private const RESP_INVALID_VALUE   = $84;
    private const RESP_INVALID_PASS    = $85;
    {$endregion}
    
    {$region Override}
    protected procedure OnFormClosing(e: FormClosingEventArgs); override;
    begin
      Disconnect();
      inherited OnFormClosing(e);
    end;
    {$endregion}
    
    {$region Routines}
    private procedure ConnectUI(connect: boolean);
    begin
      _PortSelect.Enabled     := not connect;
      _PortsUpdate.Enabled    := not connect;
      _PortConnect.Enabled    := not connect;
      _PortDisconnect.Enabled := connect;
      _ReadBox.Enabled        := connect;
      _WriteBox.Enabled       := connect;
      _UpgradeBox.Enabled     := connect;
    end;
    
    private procedure AccessUI(access: boolean);
    begin
      _PortBox.Enabled    := not access;
      _ReadBox.Enabled    := not access;
      _WriteBox.Enabled   := not access;
      _UpgradeBox.Enabled := not access;
    end;
    
    private procedure LogAppendAsync(text: string; color: System.Drawing.Color; newline: boolean := true) := Invoke(() -> _Log.Append(text, color, newline));
    
    private procedure LogErrorAsync(command: string; status: ResponseStatus) := Invoke(() -> _Log.Append($'{command} ErrorCode = {status}', Color.Red));
    
    private function Crc8Calculate(data: array of byte; length: integer := -1): byte;
    begin
      if length = -1 then
        length := data.Length;
      
      result := 0;
      
      for var i := 0 to length-1 do
        result += data[i];
      
      result := (not result) + 1;
    end;
    
    private procedure PacketEncode(buffer: array of byte; index: integer; params data: array of byte);
    begin
      for var i := 0 to data.Length-1 do
        begin
          var d := data[i];
          
          buffer[index] := ENCODE_OFFSET + (d shr 4);   index += 1;
          buffer[index] := ENCODE_OFFSET + (d and $0F); index += 1;
        end;
    end;
    
    private function PacketDecode(packet: array of byte; length: integer): array of byte;
    begin
      result := new byte[length];
      
      var index := 0;
      
      for var i := 0 to length-1 do
        begin
          var h := packet[index] - ENCODE_OFFSET; index += 1;
          var l := packet[index] - ENCODE_OFFSET; index += 1;
          
          result[i] := (h shl 4) or l;
        end;
    end;
    
    private function TransferBytes(data: array of byte; count: integer := 1; timeout: integer := 50): Response;
    begin
      _Uart.DiscardInBuffer();
      _Uart.DiscardOutBuffer();
      
      _Uart.Write(data, 0, data.Length);
      
      var t := DateTime.Now;
      
      repeat
        if _Uart.BytesToRead >= count then
          break;
      until (DateTime.Now - t).TotalMilliseconds > timeout;
      
      var len := Math.Min(count, _Uart.BytesToRead);
      
      if len = 0 then
        exit(new Response(ResponseStatus.Timedout, nil));
      
      data := new byte[len];
      _Uart.Read(data, 0, len);
      
      if len = count then
        exit(new Response(ResponseStatus.Success, data));
      
      var status: ResponseStatus;
      
      if len = 1 then
        case data[0] of
          RESP_INVALID_COMMAND: status := ResponseStatus.InvalidCommand;
          RESP_KEY_QUEUE_EMPTY: status := ResponseStatus.KeyQueueEmpty;
          RESP_INVALID_LENGTH : status := ResponseStatus.InvalidLength;
          RESP_INVALID_VALUE  : status := ResponseStatus.InvalidValue;
          RESP_INVALID_PASS   : status := ResponseStatus.InvalidPass;
          else                  status := ResponseStatus.ResponseUnknownCode;
        end
      else
        status := ResponseStatus.ResponseInvalidLength;
      
      result := new Response(status, data);
    end;
    
    private function TransferByte(data: byte; count: integer := 1; timeout: integer := 50) := TransferBytes(new byte[1](data), count, timeout);
    
    private procedure ResetKeyQueue();
    begin
      var status := TransferByte(CMD_RESET_QUEUE).Status;
      
      if status <> ResponseStatus.Success then
        LogErrorAsync('CMD_RESET_QUEUE', status);
    end;
    
    private function GetKeyCount(): integer;
    begin
      var response := TransferByte(CMD_GET_COUNT_KEY, 2);
      var status   := response.Status;
      
      if status = ResponseStatus.Success then
        begin
          var cnt := response[0];
          var crc := not response[1];
          
          if cnt = crc then
            exit(cnt);
          
          status := ResponseStatus.ResponseInvalidCrc;
        end;
      
      LogErrorAsync('CMD_GET_COUNT_KEY', status);
      
      result := -1;
    end;
    
    private function GetNextKey(): RfidKey;
    begin
      var response := TransferByte(CMD_GET_NEXT_KEY, ENCODE_BYTE_LENGTH*(RfidKey.RFID_KEY_LENGTH+ENCODE_CRC_LENGTH));
      var status   := response.Status;
      var command  := 'CMD_GET_NEXT_KEY';
      
      if status = ResponseStatus.Success then
        begin
          var packet := PacketDecode(response.Data, RfidKey.RFID_KEY_LENGTH+ENCODE_CRC_LENGTH);
          var crc    := Crc8Calculate(packet, RfidKey.RFID_KEY_LENGTH);
          
          if packet[RfidKey.RFID_KEY_LENGTH] = crc then
            begin
              status := TransferByte(crc).Status;
              
              if status = ResponseStatus.Success then
                exit(new RfidKey(packet));
              
              command := 'SEND_ACK';
            end
          else
            status := ResponseStatus.ResponseInvalidCrc;
        end;
      
      LogErrorAsync(command, status);
      
      result := nil;
    end;
    
    private function StartBootloader(): boolean;
    begin
      StopReadTask();
      
      LogAppendAsync('Start bootloader ... ', Color.Blue, false);
      
      var status  := TransferByte(CMD_START_BOOTLOADER).Status;
      var command := 'CMD_START_BOOTLOADER';
      
      if status = ResponseStatus.Success then
        begin
          status := TransferBytes(new byte[2](CMD_BOOTLOADER_PASS and $FF, CMD_BOOTLOADER_PASS shr 8), 1, 300).Status;
          
          if status = ResponseStatus.Success then
            begin
              LogAppendAsync('ok.', Color.Green);
              exit(true);
            end;
          
          command := 'BOOT_PASS_ACK';
        end;
      
      LogErrorAsync(command, status);
    end;
    
    private procedure StopReadTask();
    begin
      if _ReadTaskState = TaskState.Running then
        begin
          LogAppendAsync('Read Stop ... ', Color.Blue, false);
          
          _ReadTaskState := TaskState.Stoping;
          
          while _ReadTaskState <> TaskState.Completed do
            Thread.Sleep(20);
        end;
    end;
    
    private procedure Disconnect();
    begin
      StopReadTask();
      
      LogAppendAsync('Disconnect ... ', Color.Blue, false);
      
      if _Uart.IsOpen then
        _Uart.Close();
      
      LogAppendAsync('ok.', Color.Green);
      
      ConnectUI(false);
    end;
    
    private procedure KeyValuePaste(tb: TextBox);
    begin
      if Clipboard.ContainsText() then
        begin
          var text := Clipboard.GetText().ToUpper();
          
          if Regex.IsMatch(text, '^[0-9A-F]{1,10}$') then
            begin
              var ss := tb.SelectionStart;
                  
              tb.Text           := text;
              tb.SelectionStart := ss + text.Length;
            end;
        end;
    end;
    {$endregion}
    
    {$region Tasks}
    private procedure KeyReadTask();
    begin
      LogAppendAsync('ok.', Color.Green);
      
      ResetKeyQueue();
      
      repeat
        var count := GetKeyCount();
        
        if count > 0 then
          for var i := 0 to count-1 do
            begin
              var key := GetNextKey();
              
              if key <> nil then
                begin
                  var line := String.Format('raw: {0:X2}{1:X8} vid: {0:X2} num: {1}', key.Vendor, key.Number);
                  LogAppendAsync(line, Color.DarkMagenta);
                end;
            end;
        
        Thread.Sleep(50);
      until not (_ReadTaskState = TaskState.Running);
      
      _ReadTaskState := TaskState.Completed;
      
      Invoke(() -> 
        begin
          _Log.Append('ok.', Color.Green);
          _KeyReadStart.Enabled := true;
        end
      );
    end;
    
    private procedure KeyWriteTask(&type: RfidKeyType; keys: array of RfidKey);
    begin
      StopReadTask();
      
      var count  := keys.Length;
      var packet := new byte[1+ENCODE_BYTE_LENGTH*(count*RfidKey.RFID_KEY_LENGTH+ENCODE_CRC_LENGTH)];
      
      var index := 1;
      var crc   := 0;
      for var i := 0 to count-1 do
        begin
          var key := keys[i].ToByteArray();
          PacketEncode(packet, index, key);
          crc += Crc8Calculate(key);
          index += ENCODE_BYTE_LENGTH*RfidKey.RFID_KEY_LENGTH;
        end;
      PacketEncode(packet, index, crc);
      packet[0] := (&type = RfidKeyType.EM4x05 ? CMD_WRITE_EM4X05 : CMD_WRITE_T55X7) or (count and CMD_WRITE_KEY_MASK);
      
      LogAppendAsync('Write key ... ', Color.Blue, false);
      
      var status := TransferBytes(packet, 1, 2500).Status;
      
      if status = ResponseStatus.Success then
        LogAppendAsync('ok.', Color.Green)
      else
        LogErrorAsync(&type = RfidKeyType.EM4x05 ? 'CMD_WRITE_EM4X05' : 'CMD_WRITE_T55X7', status);
      
      Invoke(() -> AccessUI(false));
    end;
    
    private procedure BootStartTask();
    begin
      if StartBootloader() then
        begin
          Disconnect();
          
          LogAppendAsync('Now you can close this program and use the firmware download utility.' +
                         ' The device will remain in firmware update mode until the download is ' +
                         'complete or until a power cycle occurs.', Color.Black);
        end
      else
        Invoke(() -> AccessUI(false));
    end;
    
    private procedure FirmwareUploadTask(fname: string);
    begin
      if StartBootloader() then
        begin
          Disconnect();
          
          var args := $'-p={_Uart.PortName} -d=9600 -b=32 -c=4096 -s=0xF00 -k=backup.hex -f="{fname}"';
          var info := new ProcessStartInfo('picboot.exe', args);
          info.CreateNoWindow         := true;
          info.UseShellExecute        := false;
          info.RedirectStandardOutput := true;
          info.StandardOutputEncoding := Encoding.Default;
          
          LogAppendAsync($'Execute: picboot.exe {args}', Color.Black);
          
          var proc: Process;
          try
            proc := Process.Start(info);
            
            var line := '';
            
            repeat
              var c := proc.StandardOutput.Read();
          
              if c <> -1 then
                begin
                  var ch := char(c);
                  
                  if ch = '#' then
                    LogAppendAsync('#', Color.DarkMagenta, false)
                  else
                    begin
                      line += ch;
                  
                      if (ch = #10) or line.EndsWith('| ') then
                        begin
                          LogAppendAsync('picboot: ', Color.Black, false);
                          LogAppendAsync(line, Color.Blue, false);
                          line := '';
                        end;
                    end;
                end;
            until proc.StandardOutput.EndOfStream;
          except on ex: Exception do
            LogAppendAsync($'Execute error: {ex.Message}.', Color.Red);
          end;
          
          if proc <> nil then
            proc.Dispose(); 
          
          Invoke(() -> begin _PortBox.Enabled := true; end);
        end
      else
        Invoke(() -> AccessUI(false));
    end;
    {$endregion}
    
    {$region Handlers}
    private procedure PortsUpdateClick(sender: object; e: EventArgs);
    begin
      _PortConnect.Enabled := false;
      
      _PortSelect.Items.Clear();
      foreach var port in SerialPort.GetPortNames() do
        _PortSelect.Items.Add(port);
      
      if _PortSelect.Items.Count > 0 then
        _PortSelect.SelectedIndex := 0;
    end;
    
    private procedure PortSelectSelectedIndexChanged(sender: object; e: EventArgs) := _PortConnect.Enabled := _PortSelect.SelectedIndex > -1;
    
    private procedure PortConnectClick(sender: object; e: EventArgs);
    begin
      _Log.Append('Connect ... ', Color.Blue, false);
      
      _Uart.PortName := _PortSelect.SelectedItem.ToString();
          
      try
        _Uart.Open();
      except
        _Log.Append($'Port {_Uart.PortName} open fail.', Color.Red);
      end;
      
      if _Uart.IsOpen then
        begin
          ConnectUI(true);
          _Log.Append('ok.', Color.Green);
        end;
    end;
    
    private procedure PortDisconnectClick(sender: object; e: EventArgs) := Disconnect();
    
    private procedure KeyReadStartClick(sender: object; e: EventArgs);
    begin
      _KeyReadStart.Enabled := false;
      _KeyReadStop.Enabled  := true;
      
      _Log.Append('Read Start ... ', Color.Blue, false);
      _ReadTaskState := TaskState.Running;
      Task.Factory.StartNew(KeyReadTask);
    end;
    
    private procedure KeyReadStopClick(sender: object; e: EventArgs);
    begin
      _KeyReadStop.Enabled := false;
      _ReadTaskState       := TaskState.Stoping;
      _Log.Append('Read Stop ... ', Color.Blue, false);
    end;
    
    private procedure KeyTypeCheckedChanged(sender: object; e: EventArgs);
    begin
      var allow := _TypeT55X7.Checked;
      
      _KeySelect[1].Enabled := allow;
      _KeySelect[2].Enabled := allow;
      _KeyValues[1].Enabled := allow and _KeySelect[1].Checked;
      _KeyValues[2].Enabled := allow and _KeySelect[2].Checked;
      
      _KeyWrite.Enabled := allow and _KeyValues[0].Enabled or _KeySelect[0].Checked;
    end;
    
    private procedure KeyWriteClick(sender: object; e: EventArgs);
    begin
      var &type := _TypeEM4X05.Checked ? RfidKeyType.EM4x05 : RfidKeyType.T55x7;
      var count := _KeyValues[2].Enabled ? 3 : (_KeyValues[1].Enabled ? 2 : 1);
      
      var keys := new RfidKey[count];
      for var i := 0 to count-1 do
        begin
          var image := _KeyValues[i].Text;
          
          if Regex.IsMatch(image, '^[0-9A-F]{10}$') then
            keys[i] := new RfidKey(image)
          else
            begin
              _Log.Append($'Key#{i+1} parse error.', Color.Red);
              exit;
            end;
        end;
      
      AccessUI(true);
      
      Task.Factory.StartNew(() -> KeyWriteTask(&type, keys));
    end;
    
    private procedure KeySelectCheckedChanged(sender: object; e: EventArgs);
    begin
      var allow := _TypeT55x7.Checked;
      
      _KeyValues[0].Enabled := _KeySelect[0].Checked or (_KeySelect[1].Checked or _KeySelect[2].Checked) and allow;
      _KeyValues[1].Enabled := (_KeySelect[1].Checked or _KeySelect[2].Checked) and allow;
      _KeyValues[2].Enabled := _KeySelect[2].Checked and allow;
      
      _KeyWrite.Enabled := _KeyValues[0].Enabled;
    end;
    
    private procedure KeyValuesMenuCopyClick(sender: object; e: EventArgs) := ((sender as ToolStripMenuItem).Tag as TextBox).Copy();
    
    private procedure KeyValuesMenuPasteClick(sender: object; e: EventArgs) := KeyValuePaste((sender as ToolStripMenuItem).Tag as TextBox);
    
    private procedure KeyValuesKeyPress(sender: object; e: KeyPressEventArgs);
    begin
      var ch := e.KeyChar.ToUpper();
      
      case ch of
        'Ф': ch := 'A';
        'И': ch := 'B';
        'С': ch := 'C';
        'В': ch := 'D';
        'У': ch := 'E';
        'А': ch := 'F';
      end;
      
      if ch in ['0'..'9', 'A'..'F', #8] then
        e.KeyChar := ch
      else
        e.Handled := true;
    end;
    
    private procedure KeyValuesKeyDown(sender: object; e: KeyEventArgs);
    begin
      if e.Control then
        begin
          var tb := sender as TextBox;
          
          if e.KeyCode = Keys.C then
            tb.Copy()
          else if e.KeyCode = Keys.V then
            KeyValuePaste(tb)
          else if e.KeyCode = Keys.X then
            tb.Cut();
        end;
    end;
    
    private procedure BootStartClick(sender: object; e: EventArgs);
    begin
      AccessUI(true);
      
      Task.Factory.StartNew(BootStartTask);
    end;
    
    private procedure FirmwareSelectClick(sender: object; e: EventArgs);
    begin
      var dialog         := new OpenFileDialog();
      dialog.Filter      := 'IntelHex file (*.hex)|*.hex';
      dialog.DefaultExt  := 'hex';
      dialog.Multiselect := false;
      
      if dialog.ShowDialog() = System.Windows.Forms.DialogResult.OK then
        _FirmwareFname.Text := dialog.FileName;
      
      dialog.Dispose();
    end;
    
    private procedure FirmwareUploadClick(sender: object; e: EventArgs);
    begin
      var fname := _FirmwareFname.Text;
      
      if &File.Exists(fname) then
        begin
          AccessUI(true);
          
          Task.Factory.StartNew(() -> FirmwareUploadTask(fname));
        end
      else
        _Log.Append($'File "{fname}" not found.', Color.Red);
    end;
    {$endregion}
    
    {$region Ctors}
    public constructor ();
    begin
      {$region Form}
      MinimumSize   := new System.Drawing.Size(516, 408);
      Size          := new System.Drawing.Size(516, 408);
      Text          := 'RFID@125kHz Read Write';
      Icon          := Resources.Icon('icon.ico');
      StartPosition := FormStartPosition.CenterScreen;
      {$endregion}
      
      {$region Port}
      _PortBox          := new GroupBox();
      _PortBox.Size     := new System.Drawing.Size(175, 50);
      _PortBox.Location := new System.Drawing.Point(5, 5);
      _PortBox.Text     := 'Port';
      Controls.Add(_PortBox);
      
      _PortsUpdate          := new Button();
      _PortsUpdate.Size     := new System.Drawing.Size(24, 23);
      _PortsUpdate.Location := new System.Drawing.Point(5, 19);
      _PortsUpdate.Image    := Resources.Image('refresh.png');
      _PortsUpdate.TabStop  := false;
      _PortsUpdate.Click    += PortsUpdateClick;
      _PortBox.Controls.Add(_PortsUpdate);
      
      _PortSelect                      := new ComboBox();
      _PortSelect.Size                 := new System.Drawing.Size(80, 20);
      _PortSelect.Location             := new System.Drawing.Point(33, 20);
      _PortSelect.DropDownStyle        := ComboBoxStyle.DropDownList;
      _PortSelect.TabStop              := false;
      _PortSelect.SelectedIndexChanged += PortSelectSelectedIndexChanged;
      _PortBox.Controls.Add(_PortSelect);
      
      _PortConnect          := new Button();
      _PortConnect.Size     := new System.Drawing.Size(24, 23);
      _PortConnect.Location := new System.Drawing.Point(118, 19);
      _PortConnect.Image    := Resources.Image('connect.png');
      _PortConnect.TabStop  := false;
      _PortConnect.Click    += PortConnectClick;
      _PortBox.Controls.Add(_PortConnect);
      
      _PortDisconnect          := new Button();
      _PortDisconnect.Size     := new System.Drawing.Size(24, 23);
      _PortDisconnect.Location := new System.Drawing.Point(146, 19);
      _PortDisconnect.Image    := Resources.Image('disconnect.png');
      _PortDisconnect.TabStop  := false;
      _PortDisconnect.Click    += PortDisconnectClick;
      _PortBox.Controls.Add(_PortDisconnect);
      {$endregion}
      
      {$region Read}
      _ReadBox          := new GroupBox();
      _ReadBox.Size     := new System.Drawing.Size(175, 50);
      _ReadBox.Location := new System.Drawing.Point(5, 60);
      _ReadBox.Text     := 'Read';
      Controls.Add(_ReadBox);
      
      _KeyReadStart          := new Button();
      _KeyReadStart.Size     := new System.Drawing.Size(80, 23);
      _KeyReadStart.Location := new System.Drawing.Point(5, 20);
      _KeyReadStart.TabStop  := false;
      _KeyReadStart.Text     := 'Start';
      _KeyReadStart.Click    += KeyReadStartClick;
      _ReadBox.Controls.Add(_KeyReadStart);
      
      _KeyReadStop          := new Button();
      _KeyReadStop.Size     := new System.Drawing.Size(80, 23);
      _KeyReadStop.Location := new System.Drawing.Point(90, 20);
      _KeyReadStop.TabStop  := false;
      _KeyReadStop.Text     := 'Stop';
      _KeyReadStop.Click    += KeyReadStopClick;
      _ReadBox.Controls.Add(_KeyReadStop);
      {$endregion}
      
      {$region Write}
      _WriteBox          := new GroupBox();
      _WriteBox.Size     := new System.Drawing.Size(175, 145);
      _WriteBox.Location := new System.Drawing.Point(5, 115);
      _WriteBox.Text     := 'Write';
      Controls.Add(_WriteBox);
      
      _TypeT55X7                := new RadioButton();
      _TypeT55X7.Size           := new System.Drawing.Size(70, 15);
      _TypeT55X7.Location       := new System.Drawing.Point(7, 20);
      _TypeT55X7.Text           := 'T55X7';
      _TypeT55X7.TabStop        := false;
      _TypeT55X7.CheckedChanged += KeyTypeCheckedChanged;
      _WriteBox.Controls.Add(_TypeT55X7);
      
      _TypeEM4X05                := new RadioButton();
      _TypeEM4X05.Size           := new System.Drawing.Size(70, 15);
      _TypeEM4X05.Location       := new System.Drawing.Point(7, 36);
      _TypeEM4X05.Text           := 'EM4X05';
      _TypeEM4X05.TabStop        := false;
      _TypeEM4X05.CheckedChanged += KeyTypeCheckedChanged;
      _WriteBox.Controls.Add(_TypeEM4X05);
      
      _KeyWrite          := new Button();
      _KeyWrite.Size     := new System.Drawing.Size(80, 23);
      _KeyWrite.Location := new System.Drawing.Point(90, 23);
      _KeyWrite.Text     := 'Write';
      _KeyWrite.TabStop  := false;
      _KeyWrite.Click    += KeyWriteClick;
      _WriteBox.Controls.Add(_KeyWrite);
      
      _KeySelect := new CheckBox[3];
      _KeyValues := new TextBox[3];
      for var i := 0 to 2 do
        begin
          _KeySelect[i]                := new CheckBox();
          _KeySelect[i].Size           := new System.Drawing.Size(15, 15);
          _KeySelect[i].Location       := new System.Drawing.Point(7, 65+25*i);
          _KeySelect[i].CheckedChanged += KeySelectCheckedChanged;
          _KeySelect[i].Tag            := i;
          _KeySelect[i].TabStop        := false;
          _WriteBox.Controls.Add(_KeySelect[i]);
          
          _KeyValues[i]                  := new TextBox();
          _KeyValues[i].Size             := new System.Drawing.Size(145, 20);
          _KeyValues[i].Location         := new System.Drawing.Point(25, 60+25*i);
          _KeyValues[i].Font             := new System.Drawing.Font('Consolas', 10, FontStyle.Regular, GraphicsUnit.Point);
          _KeyValues[i].MaxLength        := 10;
          _KeyValues[i].TextAlign        := HorizontalAlignment.Center;
          _KeyValues[i].TabIndex         := i;
          _KeyValues[i].Enabled          := false;
          _KeyValues[i].KeyPress         += KeyValuesKeyPress;
          _KeyValues[i].KeyDown          += KeyValuesKeyDown;
          _WriteBox.Controls.Add(_KeyValues[i]);
          
          var _KeyValuesMenu := new System.Windows.Forms.ContextMenuStrip();
          
          var _KeyValuesMenuCopy   := new ToolStripMenuItem();
          _KeyValuesMenuCopy.Text  := 'Copy';
          _KeyValuesMenuCopy.Image := Resources.Image('copy.png');
          _KeyValuesMenuCopy.Click += KeyValuesMenuCopyClick;
          _KeyValuesMenuCopy.Tag   := _KeyValues[i];
          _KeyValuesMenu.Items.Add(_KeyValuesMenuCopy);
          
          var _KeyValuesMenuPaste   := new ToolStripMenuItem();
          _KeyValuesMenuPaste.Text  := 'Paste';
          _KeyValuesMenuPaste.Image := Resources.Image('paste.png');
          _KeyValuesMenuPaste.Click += KeyValuesMenuPasteClick;
          _KeyValuesMenuPaste.Tag   := _KeyValues[i];
          _KeyValuesMenu.Items.Add(_KeyValuesMenuPaste);
          
          _KeyValues[i].ContextMenuStrip := _KeyValuesMenu;
        end;
      {$endregion}
      
      {$region Log}
      _Log          := new LogBox();
      _Log.Size     := new System.Drawing.Size(310, 353);
      _Log.Location := new System.Drawing.Point(185, 11);
      _Log.Anchor   := AnchorStyles.Left or AnchorStyles.Top or AnchorStyles.Right or AnchorStyles.Bottom;
      _Log.TabStop  := false;
      Controls.Add(_Log);
      {$endregion}
      
      {$region Upgrade}
      _UpgradeBox          := new GroupBox();
      _UpgradeBox.Size     := new System.Drawing.Size(175, 100);
      _UpgradeBox.Location := new System.Drawing.Point(5, 265);
      _UpgradeBox.Text     := 'Upgrade';
      Controls.Add(_UpgradeBox);
      
      _BootStart          := new Button();
      _BootStart.Size     := new System.Drawing.Size(165, 23);
      _BootStart.Location := new System.Drawing.Point(5, 15);
      _BootStart.TabStop  := false;
      _BootStart.Text     := 'Start Bootloader';
      _BootStart.Click    += BootStartClick;
      _UpgradeBox.Controls.Add(_BootStart);
      
      _FirmwareFname          := new TextBox();
      _FirmwareFname.Size     := new System.Drawing.Size(140, 23);
      _FirmwareFname.Location := new System.Drawing.Point(6, 43);
      _FirmwareFname.TabStop  := false;
      _UpgradeBox.Controls.Add(_FirmwareFname);
      
      _FirmwareSelect          := new Button();
      _FirmwareSelect.Size     := new System.Drawing.Size(24, 22);
      _FirmwareSelect.Location := new System.Drawing.Point(146, 42);
      _FirmwareSelect.Image    := Resources.Image('open.png');
      _FirmwareSelect.TabStop  := false;
      _FirmwareSelect.Click    += FirmwareSelectClick;
      _UpgradeBox.Controls.Add(_FirmwareSelect);
      
      _FirmwareUpload          := new Button();
      _FirmwareUpload.Size     := new System.Drawing.Size(165, 23);
      _FirmwareUpload.Location := new System.Drawing.Point(5, 68);
      _FirmwareUpload.TabStop  := false;
      _FirmwareUpload.Text     := 'Firmware Upload';
      _FirmwareUpload.Click    += FirmwareUploadClick;
      _UpgradeBox.Controls.Add(_FirmwareUpload);
      {$endregion}
      
      {$region Init}
      _PortConnect.Enabled    := false;
      _PortDisconnect.Enabled := false;
      _TypeT55X7.Checked      := true;
      _KeyWrite.Enabled       := false;
      _KeyReadStop.Enabled    := false;
      _ReadBox.Enabled        := false;
      _WriteBox.Enabled       := false;
      _UpgradeBox.Enabled     := false;
      
      PortsUpdateClick(_PortsUpdate, EventArgs.Empty);
      
      _ReadTaskState := TaskState.Completed;
      
      _Uart          := new SerialPort();
      _Uart.BaudRate := 9600;
      _Uart.DataBits := 8;
      _Uart.Parity   := Parity.None;
      _Uart.StopBits := StopBits.One;
      {$endregion}
    end;
    {$endregion}
  end;


end.