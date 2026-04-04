unit UPicBootloader;


uses System;
uses System.IO;
uses System.IO.Ports;


type
  ResponseStatus =
  (
    Success          = 0,
    Timedout         = 1,
    InvalidDataByte  = 2,
    ByteReadError    = 3,
    CommandExecError = 4,
    InvalidResponse  = 5,
    InvalidArgument  = 6,
    VerifyFail       = 7
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
    
    public property Data: array of byte read (_Data);
    
    public property DataBytesCount: integer read (_Data <> nil ? _Data.Length : 0);
    
    public property DataWordsCount: integer read (_Data <> nil ? _Data.Length div 2 : 0);
    
    public property Bytes[i: integer]: byte read ((_Data <> nil) and (i < _Data.Length) ? _Data[i] : 0); default;
    
    public property Words[i: integer]: byte read ((_Data <> nil) and ((2*i) < _Data.Length) ? _Data[2*i] : 0);
    {$endregion}
    
    {$region Methods}
    public function GetWordData(): array of word;
    begin
      result := new word[DataWordsCount];
      
      for var i := 0 to result.Length-1 do
        result[i] := BitConverter.ToUInt16(_Data, 2*i);
    end;
    {$endregion}
  end;
  
  PicBootloader = class(IDisposable)
    {$region Fields}
    protected _Uart     : SerialPort;
    protected _Timeout  : integer;
    protected _FlashSize: integer;
    protected _BootStart: integer;
    protected _BlockSize: integer;
    {$endregion}
    
    {$region Consts}
    private const HOST_RESP_SUCCESS     = $50;
    private const HOST_RESP_ERROR       = $E0;
    private const HOST_RESP_INVALID_ADR = $A0;
    private const CMD_FLASH_READ_BLOCK  = $10;
    private const CMD_FLASH_FILL_BLOCK  = $20;
    private const CMD_FLASH_ERASE_ROW   = $30;
    private const CMD_ECHO_PRESENCE     = $40;
    private const CMD_LOADING_END       = $80;
    {$endregion}
    
    {$region Ctors}
    public constructor (port: string; baud: integer := 9600);
    begin
      _Uart          := new SerialPort();
      _Uart.PortName := port;
      _Uart.BaudRate := baud;
      _Uart.DataBits := 8;
      _Uart.Parity   := Parity.None;
      _Uart.StopBits := StopBits.One;
      
      _Timeout   := 5000;
      _FlashSize := 4096;
      _BootStart := $F00;
      _BlockSize := 32;
    end;
    {$endregion}
    
    {$region Properties}
    public property Port     : string read (_Uart.PortName) write _Uart.PortName := value;
    
    public property Baudrate : integer read (_Uart.BaudRate) write _Uart.BaudRate := value;
    
    public property Timeout  : integer read _Timeout   write _Timeout;
    
    public property FlashSize: integer read _FlashSize write _FlashSize;
    
    public property BootStart: integer read _BootStart write _BootStart;
    
    public property BlockSize: integer read _BlockSize write _BlockSize;
    
    public property IsOpen   : boolean read (_Uart.IsOpen);
    {$endregion}
    
    {$region Methods}
    public function Open(): boolean;
    begin
      try
        _Uart.Open();
      except
      end;
      
      result := _Uart.IsOpen;
    end;
    
    public procedure Close();
    begin
      if _Uart.IsOpen then
        _Uart.Close();
    end;
    
    public procedure Dispose();
    begin
      Close();
      _Uart.Dispose();
    end;
    
    private function ReceiveBytes(count: integer := 1): Response;
    begin
      var data := new byte[count];
      
      var p := 0;
      var t := DateTime.Now;
      while (p < (2*count)) and ((DateTime.Now - t).TotalMilliseconds < _Timeout) do
        if _Uart.BytesToRead > 0 then
          begin
            var temp := _Uart.ReadByte();
            
            if (temp and $FFFFFF00) = 0 then
              begin
                if (temp and $F0) = 0 then
                  begin
                    if (p and 1) = 0 then
                      data[p shr 1] := temp
                    else
                      data[p shr 1] += temp shl 4;
                    
                    p += 1;
                  end
                else
                  exit(new Response(ResponseStatus.InvalidDataByte, nil));
              end
            else
              exit(new Response(ResponseStatus.ByteReadError, nil));
          end;
      
      result := new Response(p = (2*count) ? ResponseStatus.Success : ResponseStatus.Timedout, data);
    end;
    
    private function CommandResponseTranslate(resp: Response): ResponseStatus;
    begin
      if resp.Status = ResponseStatus.Success then
        case resp[0] of
          HOST_RESP_SUCCESS    : result := ResponseStatus.Success;
          HOST_RESP_ERROR      : result := ResponseStatus.CommandExecError;
          HOST_RESP_INVALID_ADR: result := ResponseStatus.InvalidArgument;
          else                   result := ResponseStatus.InvalidResponse;
        end
      else
        result := resp.Status;
    end;
    
    private function SendCommand(command: byte): ResponseStatus;
    begin
      _Uart.DiscardInBuffer();
      _Uart.DiscardOutBuffer();
      _Uart.Write(new byte[1](command), 0, 1);
      
      result := CommandResponseTranslate(ReceiveBytes());
    end;
    
    private procedure SendCommandParams(command: byte; offset: word; count: word);
    begin
      var packet := new byte[1+2*4];
  
      packet[0] := command;
      
      packet[1] := (offset shr 0) and $0F;
      packet[2] := (offset shr 4) and $0F;
      offset    :=  offset shr 8;
      packet[3] := (offset shr 0) and $0F;
      packet[4] := (offset shr 4) and $0F;
      
      packet[5] := (count shr 0) and $0F;
      packet[6] := (count shr 4) and $0F;
      count     :=  count shr 8;
      packet[7] := (count shr 0) and $0F;
      packet[8] := (count shr 4) and $0F;
      
      _Uart.DiscardInBuffer();
      _Uart.DiscardOutBuffer();
      _Uart.Write(packet, 0, packet.Length);
    end;
    
    private function SendFlashBlock(data: array of word; offset: integer; count: integer): ResponseStatus;
    begin
      var packet := new byte[2*2];
      
      for var i := 0 to count-1 do
        begin
          var w := data[offset+i];
          
          packet[0] := (w shr  0) and $0F;
          packet[1] := (w shr  4) and $0F;
          packet[2] := (w shr  8) and $0F;
          packet[3] := (w shr 12) and $0F;
          
          _Uart.DiscardInBuffer();
          _Uart.DiscardOutBuffer();
          _Uart.Write(packet, 0, packet.Length);
          
          result := CommandResponseTranslate(ReceiveBytes());
          
          if result <> ResponseStatus.Success then
            exit;
        end;
    end;
    
    public function CheckPresence() := SendCommand(CMD_ECHO_PRESENCE);
    
    public function FlashReadBlock(address: integer): Response;
    begin
      SendCommandParams(CMD_FLASH_READ_BLOCK, address, _BlockSize);
      result := ReceiveBytes(2*_BlockSize);
    end;
    
    public function FlashEraseBlock(address: integer): ResponseStatus;
    begin
      SendCommandParams(CMD_FLASH_ERASE_ROW, address, 0);
      result := CommandResponseTranslate(ReceiveBytes());
    end;
    
    public function FlashWriteBlock(address: integer; data: array of word; count: integer): ResponseStatus;
    begin
      SendCommandParams(CMD_FLASH_FILL_BLOCK, address, count);
      result := CommandResponseTranslate(ReceiveBytes());
      
      if result = ResponseStatus.Success then
        begin
          result := SendFlashBlock(data, address, count);
          
          if result = ResponseStatus.Success then
            result := CommandResponseTranslate(ReceiveBytes());
        end;
    end;
    
    public function FlashRelease() := SendCommand(CMD_LOADING_END);
    {$endregion}
  end;


end.