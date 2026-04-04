unit UMainForm;


{$reference System.Drawing.dll}
{$reference System.Windows.Forms.dll}

{$resource res\icon.ico}
{$resource res\refresh.png}
{$resource res\connect.png}
{$resource res\disconnect.png}
{$resource res\paste.png}


uses System;
uses System.IO;
uses System.IO.Ports;
uses System.Text.RegularExpressions;
uses System.Globalization;
uses System.Threading;
uses System.Threading.Tasks;
uses System.Drawing;
uses System.Windows.Forms;
uses UExtensions;
uses ULogBox;


type
  TaskState = ( Running, Stoping, Completed );
  
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
    private _Uart           : SerialPort;
    private _ReadTaskState  : TaskState;
    {$endregion}
    
    {$region Constants}
    private const RFID_KEY_LENGTH      = 5;
    private const ENCODE_BYTE_LENGTH   = 2;
    private const ENCODE_CRC_LENGTH    = 1;
    private const ENCODED_KEY_LENGTH   = ENCODE_BYTE_LENGTH*(RFID_KEY_LENGTH+ENCODE_CRC_LENGTH);
    private const CMD_GET_COUNT_KEY    = $04;
    private const CMD_GET_NEXT_KEY     = $08;
    private const CMD_RESET_QUEUE      = $10;
    private const CMD_WRITE_T55X7      = $20;
    private const CMD_WRITE_EM4X05     = $40;
    private const CMD_WRITE_KEY_MASK   = $03;
    private const RESP_SUCCESS         = $00;
    private const RESP_INVALID_COMMAND = $81;
    private const RESP_KEY_QUEUE_EMPTY = $82;
    private const RESP_INVALID_LENGTH  = $83;
    private const RESP_INVALID_VALUE   = $84;
    private const ENCODE_OFFSET        = $47;
    {$endregion}
    
    {$region Override}
    protected procedure OnFormClosing(e: FormClosingEventArgs); override := Disconnect();
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
    end;
    
    private procedure AccessUI(access: boolean);
    begin
      _PortBox.Enabled  := not access;
      _ReadBox.Enabled  := not access;
      _WriteBox.Enabled := not access;
    end;
    
    private function SerialTransfer(data: array of byte; length: integer; timeout: integer := 50): array of byte;
    begin
      _Uart.DiscardInBuffer();
      _Uart.Write(data, 0, data.Length);
      
      result := nil;
      
      var t := DateTime.Now;
      while (DateTime.Now - t).TotalMilliseconds < timeout do
        if _Uart.BytesToRead >= length then
          break;
      
      var count := Math.Min(_Uart.BytesToRead, length);
      
      if count > 0 then
        begin
          result := new byte[count];
          _Uart.Read(result, 0, count);
        end;
    end;
    
    private function PacketDecode(packet: array of byte): array of byte;
    begin
      var len := packet.Length div ENCODE_BYTE_LENGTH;
      result := new byte[len];
      for var i := 0 to len-1 do
        result[i] := ((packet[ENCODE_BYTE_LENGTH*i+0] - ENCODE_OFFSET) shl 4) or (packet[ENCODE_BYTE_LENGTH*i+1] - ENCODE_OFFSET);
    end;
    
    private function PacketCrc(packet: array of byte; length: integer): byte;
    begin
      result := 0;
      for var i := 0 to length-1 do
        result += packet[i];
      result := (not result) + 1;
    end;
    
    private function PacketEncode(packet: array of byte): array of byte;
    begin
      result := new byte[ENCODE_BYTE_LENGTH*(packet.Length+ENCODE_CRC_LENGTH)];
      for var i := 0 to packet.Length-1 do
        begin
          result[ENCODE_BYTE_LENGTH*i+0] := ENCODE_OFFSET + (packet[i] shr 4);
          result[ENCODE_BYTE_LENGTH*i+1] := ENCODE_OFFSET + (packet[i] and $0F);
        end;
      var crc := PacketCrc(packet, packet.Length);
      result[ENCODE_BYTE_LENGTH*packet.Length+0] := ENCODE_OFFSET + (crc shr 4);
      result[ENCODE_BYTE_LENGTH*packet.Length+1] := ENCODE_OFFSET + (crc and $0F);
    end;
    
    private procedure ResetKeyQueue();
    begin
      var response := SerialTransfer(new byte[](CMD_RESET_QUEUE), 1, 50);
      if response <> nil then
        begin
          if response.Length = 1 then
            begin
              var code := response[0];
              
              if code <> RESP_SUCCESS then
                Invoke(() -> _Log.Append($'CMD_RESET_QUEUE erorr = 0x{code:X2}', Color.Red));
            end
          else
            Invoke(() -> _Log.Append($'CMD_RESET_QUEUE response invalid length = {response.Length}', Color.Red));
        end
      else
        Invoke(() -> _Log.Append('CMD_RESET_QUEUE timedout', Color.Red));
    end;
    
    private function GetKeyCount(): integer;
    begin
      result := -1;
      
      var response := SerialTransfer(new byte[](CMD_GET_COUNT_KEY), 2, 50);
      if response <> nil then
        begin
          if response.Length = 2 then
            begin
              var cnt := response[0];
              var crc := not response[1];
              
              if cnt = crc then
                result := cnt
              else
                Invoke(() -> _Log.Append($'CMD_GET_COUNT_KEY response invalid crc: {cnt} != {crc}', Color.Red));
            end
          else
            Invoke(() -> _Log.Append($'CMD_GET_COUNT_KEY response invalid length = {response.Length}', Color.Red));
        end
      else
        Invoke(() -> _Log.Append('CMD_GET_COUNT_KEY timedout', Color.Red));
    end;
    
    private function GetNextKey(): array of byte;
    begin
      result := nil;
      
      var response := SerialTransfer(new byte[](CMD_GET_NEXT_KEY), ENCODED_KEY_LENGTH, 50);
      if response <> nil then
        begin
          if response.Length = ENCODED_KEY_LENGTH then
            begin
              var packet := PacketDecode(response);
              var crc    := PacketCrc(packet, packet.Length-1);
              
              if packet[RFID_KEY_LENGTH] = crc then
                begin
                  response := SerialTransfer(new byte[](crc), 1, 50);
                  if response <> nil then
                    begin
                      if response.Length = 1 then
                        begin
                          var code := response[0];
                          
                          if code <> RESP_SUCCESS then
                            Invoke(() -> _Log.Append($'ACK erorr = 0x{code:X2}', Color.Red));
                        end
                      else
                        Invoke(() -> _Log.Append($'ACK response invalid length = {response.Length}', Color.Red));
                    end
                  else
                    Invoke(() -> _Log.Append('ACK timedout', Color.Red));
                  
                  result := new byte[RFID_KEY_LENGTH];
                  &Array.Copy(packet, result, RFID_KEY_LENGTH);
                end
              else
                Invoke(() -> _Log.Append($'CMD_GET_NEXT_KEY response invalid crc: {packet[RFID_KEY_LENGTH]} != {crc}', Color.Red));
            end
          else
            Invoke(() -> _Log.Append($'CMD_GET_NEXT_KEY response invalid length = {response.Length}', Color.Red));
        end
      else
        Invoke(() -> _Log.Append('CMD_GET_NEXT_KEY timedout', Color.Red));
    end;
    
    private procedure StopReadTask();
    begin
      if _ReadTaskState = TaskState.Running then
        begin
          _Log.Append('Read Stop ... ', Color.Blue, false);
          
          _ReadTaskState := TaskState.Stoping;
          
          while _ReadTaskState <> TaskState.Completed do
            Thread.Sleep(20);
        end;
    end;
    
    private procedure Disconnect();
    begin
      StopReadTask();
      
      _Log.Append('Disconnect ... ', Color.Blue, false);
      
      if _Uart.IsOpen then
        _Uart.Close();
      
      _Log.Append('ok.', Color.Green);
      
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
      Invoke(() -> _Log.Append('ok.', Color.Green));
      
      ResetKeyQueue();
      
      repeat
        var count := GetKeyCount();
        
        if count > 0 then
          for var i := 0 to count-1 do
            begin
              var key := GetNextKey();
              
              if key <> nil then
                begin
                  var val := longword(0);
                  for var j := 1 to RFID_KEY_LENGTH-1 do
                    val := (val shl 8) or key[j];
                  
                  var line := String.Format('raw: {0:X2}{1:X8} vid: {0:X2} num: {1}', key[0], val);
                  Invoke(() -> _Log.Append(line, Color.DarkMagenta));
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
    
    private procedure KeyWriteTask(command: byte; keys: array of byte);
    begin
      StopReadTask();
      
      var enkeys := PacketEncode(keys);
      var packet := new byte[1+enkeys.Length];
      packet[0] := command;
      &Array.Copy(enkeys, 0, packet, 1, enkeys.Length);
      
      Invoke(() -> _Log.Append('Write key ... ', Color.Blue, false));
      
      var CommandDesc := (command and CMD_WRITE_T55X7) = CMD_WRITE_T55X7 ? 'CMD_WRITE_T55X7' : 'CMD_WRITE_EM4X05';
      
      var response := SerialTransfer(packet, 1, 2500);
      if response <> nil then
        begin
          if response.Length = 1 then
            begin
              var code := response[0];
              
              if code = RESP_SUCCESS then
                Invoke(() -> _Log.Append('ok.', Color.Green))
              else
                Invoke(() -> _Log.Append($'{CommandDesc} erorr = 0x{code:X2}', Color.Red));
            end
          else
            Invoke(() -> _Log.Append($'{CommandDesc} response invalid length = {response.Length}', Color.Red));
        end
      else
        Invoke(() -> _Log.Append($'{CommandDesc} timedout', Color.Red));
      
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
      _Log.Append('Read Stop ... ', Color.Blue, false);
      _KeyReadStop.Enabled := false;
      _ReadTaskState       := TaskState.Stoping;
    end;
    
    private procedure KeyTypeCheckedChanged(sender: object; e: EventArgs);
    begin
      var allow := _TypeT55X7.Checked;
      _KeySelect[1].Enabled := allow;
      _KeySelect[2].Enabled := allow;
      _KeyValues[1].Enabled := allow and _KeySelect[1].Checked;
      _KeyValues[2].Enabled := allow and _KeySelect[2].Checked;
    end;
    
    private procedure KeyWriteClick(sender: object; e: EventArgs);
    begin
      var count   := _KeyValues[2].Enabled ? 3 : (_KeyValues[1].Enabled ? 2 : 1);
      var command := (_TypeEM4X05.Checked ? CMD_WRITE_EM4X05 : CMD_WRITE_T55X7) or (count and CMD_WRITE_KEY_MASK);
      
      var keys := new byte[RFID_KEY_LENGTH*count];
      for var i := 0 to count-1 do
        begin
          var image := _KeyValues[i].Text;
          
          if Regex.IsMatch(image, '^[0-9A-F]{10}$') then
            begin
              for var j := 0 to RFID_KEY_LENGTH-1 do
                keys[RFID_KEY_LENGTH*i+j] := Convert.ToByte(image.Substring(2*j, 2), 16);
            end
          else
            begin
              _Log.Append($'Key#{i+1} parse error.', Color.Red);
              exit;
            end;
        end;
      
      AccessUI(true);
      
      Task.Factory.StartNew(() -> KeyWriteTask(command, keys));
    end;
    
    private procedure KeySelectCheckedChanged(sender: object; e: EventArgs);
    begin
      _KeyValues[0].Enabled := _KeySelect[0].Checked or _KeySelect[1].Checked or _KeySelect[2].Checked;
      _KeyValues[1].Enabled := _KeySelect[1].Checked or _KeySelect[2].Checked;
      _KeyValues[2].Enabled := _KeySelect[2].Checked;
      
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
    {$endregion}
    
    {$region Ctors}
    public constructor ();
    begin
      {$region Form}
      MinimumSize   := new System.Drawing.Size(520, 305);
      Size          := new System.Drawing.Size(520, 305);
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
      _Log.Size     := new System.Drawing.Size(310, 248);
      _Log.Location := new System.Drawing.Point(185, 11);
      _Log.Anchor   := AnchorStyles.Left or AnchorStyles.Top or AnchorStyles.Right or AnchorStyles.Bottom;
      _Log.TabStop  := false;
      Controls.Add(_Log);
      {$endregion}
      
      {$region Init}
      _PortConnect.Enabled    := false;
      _PortDisconnect.Enabled := false;
      _TypeT55X7.Checked      := true;
      _KeyReadStop.Enabled    := false;
      _ReadBox.Enabled        := false;
      _WriteBox.Enabled       := false;
      
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