{$reference System.IntelHex.dll}


uses System;
uses System.IO;
uses System.IO.Ports;
uses System.Globalization;
uses System.Text.RegularExpressions;
uses System.IntelHex;
uses UBoolean;
uses UPicBootloader;


type
  BootloaderException = class(Exception)
    public constructor Create(method: string; offset: integer; resp: ResponseStatus; text: string);
    begin
      inherited Create($'{method} offset=0x{offset:X4} failed with code={resp}. {text}.');
    end;
  end;
  
  NotRespondigException = class(BootloaderException)
    public constructor (method: string; offset: integer; resp, stat: ResponseStatus);
    begin
      inherited Create(method, offset, resp, $'Firmware not responding. Status code={stat}');
    end;
  end;
  
  UnsuccessfulException = class(BootloaderException)
    public constructor(method: string; offset: integer; resp: ResponseStatus; retires: integer);
    begin
      inherited Create(method, offset, resp, $'{retires} Unsuccessful attempts were made'); 
    end;
  end;
  
  BootloaderConsole = class(PicBootloader)
    {$region Ctors}
    public constructor (port: string; baud, retries: integer; reduce, verbose: boolean);
    begin
      inherited Create(port, baud);
      
      self.MaxRetries := retries;
      self.ReduceHex  := reduce;
      self.Verbose    := verbose;
    end;
    {$endregion}
    
    {$region Properties}
    public auto property MaxRetries: integer;
    
    public auto property ReduceHex: boolean;
    
    public auto property Verbose: boolean;
    {$endregion}
    
    {$region Static}
    private static function IsEmptyBlock(dump: array of word; offset: integer; count: integer): boolean;
    begin
      for var i := offset to offset+count-1 do
        if dump[i] <> $3FFF then
          exit(false);
      result := true;
    end;
        
    private static function BlocksDiffIndex(block, ref: array of word; offset: integer; count: integer): integer;
    begin
      for var i := 0 to count-1 do
        if block[i] <> ref[offset+i] then
          exit(i);
      result := -1;
    end;
    {$endregion}
    
    {$region Methods}
    private function HandleFail(method: string; offset: integer; resp: ResponseStatus; retries: integer): integer;
    begin
      var stat := CheckPresence();
          
      if stat = ResponseStatus.Success then
        begin
          if retries < MaxRetries then
            retries += 1
          else
            raise new UnsuccessfulException(method, offset, resp, MaxRetries);
        end
      else
        raise new NotRespondigException(method, offset, resp, stat);
      
      result := retries;
    end;
    
    public function IsPresence(): boolean;
    begin
      for var i := 0 to MaxRetries-1 do
        begin
          var stat := CheckPresence();
          
          if stat = ResponseStatus.Success then
            exit(true);
        end;
      result := false;
    end;
    
    public procedure FirmwareDownload(fname: string);
    begin
      Console.Write('Reading | ');
      
      var dt := DateTime.Now;
      var no := 0;
      
      var offset  := 0;
      var retries := 0;
      
      repeat
        var resp := FlashReadBlock(offset);
        
        if resp.Status = ResponseStatus.Success then
          begin
            var block := resp.GetWordData();
            var isend := offset = (_FlashSize - _BlockSize);
            
            if ReduceHex then
              IntelHex.SaveWordsReduced(fname, block, offset, isend, $3FFF)
            else
              IntelHex.SaveWords(fname, block, offset, isend);
            
            offset  += _BlockSize;
            retries := 0;
            
            var n := (50 * offset) div _FlashSize - no;
            if n > 0 then
              begin
                Console.Write('#'*n);
                no += n;
              end;
          end
        else
          retries := HandleFail('FlashReadBlock', offset, resp.Status, retries);
      until offset >= _FlashSize;
      
      var t := (DateTime.Now - dt).TotalSeconds;
      var v := _FlashSize / (t + 1e-6);
      Console.WriteLine($' | {t:f2}s ({v:f1}w/s).');
    end;
    
    public procedure FirmwareUpload(fname: string);
    begin
      var dump := IntelHex.ReadDump
      (
        fname,
        new IntelHexSegmentInfo($0000, _FlashSize, IntelHexSegmentType.Word, $3FFF),
        new IntelHexSegmentInfo($8000, 4,          IntelHexSegmentType.Word, $3FFF),
        new IntelHexSegmentInfo($8007, 2,          IntelHexSegmentType.Word, $3FFF),
        new IntelHexSegmentInfo($F000, 256,        IntelHexSegmentType.Word, $3FFF)
      );
      
      var flash  := dump[0].Words;
      var offset := $0000;
      
      var resp   : ResponseStatus;
      var data   : Response;
      var retries: integer;
      
      var bn := 0;
      var bw := 0;
      var dt := DateTime.Now;
      
      if not Verbose then
        Console.Write('Writing | ');
      
      repeat
        if Verbose then
          Console.Write($'block{bn} at 0x{offset:X4}: ');
        
        if not IsEmptyBlock(flash, offset, _BlockSize) then
          begin
            var en := 0;
            var wn := 0;
            var rn := 0;
            
            var diff   : integer;
            var attempt: integer;
            
            repeat
              // Erase
              retries := 0;
              repeat
                resp := FlashEraseBlock(offset);
                if resp <> ResponseStatus.Success then
                  retries := HandleFail('FlashEraseBlock', offset, resp, retries);
              until resp = ResponseStatus.Success;
              en += retries;
              
              // Write
              retries := 0;
              repeat
                resp := FlashWriteBlock(offset, flash, _BlockSize);
                if resp <> ResponseStatus.Success then
                  retries := HandleFail('FlashWriteBlock', offset, resp, retries);
              until resp = ResponseStatus.Success;
              wn += retries;
              
              // Read
              retries := 0;
              repeat
                data := FlashReadBlock(offset);
                if data.Status <> ResponseStatus.Success then
                  retries := HandleFail('FlashReadBlock', offset, data.Status, retries);
              until data.Status = ResponseStatus.Success;
              rn += retries;
              
              diff := BlocksDiffIndex(data.GetWordData(), flash, offset, _BlockSize);
              if diff <> -1 then
                attempt := HandleFail($'BlocksDiffIndex, index={diff}', offset, ResponseStatus.VerifyFail, attempt);
            until diff = -1;
            
            if Verbose then
              Console.WriteLine($'E={en}, W={wn}, R={rn}, V={attempt}');
            
            bw += 1;
          end
        else
          if Verbose then
            Console.WriteLine('is empty');
        
        if Verbose then
          bn += 1
        else
          begin
            var n := (50 * offset) div _FlashSize - bn;
            if n > 0 then
              begin
                Console.Write('#'*n);
                bn += n;
              end;
          end;
        
        offset += _BlockSize;
      until offset >= _BootStart;
      
      var t := (DateTime.Now - dt).TotalSeconds;
      var v := (bw * _BlockSize) / (t + 1e-6);
      
      if not Verbose then
        Console.WriteLine($' | {t:f2}s ({v:f1}w/s).');
      
      // Target Reboot
      Console.Write('Target Reset ... ');
      retries := 0;
      repeat
        resp := FlashRelease();
        if resp <> ResponseStatus.Success then
          retries := HandleFail('FlashRelease', offset, resp, retries);
      until resp = ResponseStatus.Success;
      Console.Write('OK.');
       
      if Verbose then
        Console.WriteLine($' {t:f2}s ({v:f1}w/s).')
      else
        Console.WriteLine();
    end;
    {$endregion}
  end;


function ToInteger(image: string): integer;
begin
  if String.IsNullOrWhiteSpace(image) then
    exit(-1);
  
  image := image.Trim().ToLower();
  
  if not Int32.TryParse(image, NumberStyles.Integer, nil, result) then
    begin
      if image.StartsWith('0x') then
        image := image.Substring(2)
      else if image.StartsWith('$') then
        image := image.Substring(1)
      else if image.EndsWith('h') then
        image := image.TrimEnd('h');
      
      if not Int32.TryParse(image, NumberStyles.HexNumber, nil, result) then
        result := -1;
    end;
end;

procedure ErrorExit(text: string);
begin
  Console.WriteLine(text);
  Environment.Exit(0);
end;

function VerifyFileName(fname: string): string;
begin
  var info: FileInfo;
  
  try
    info := new FileInfo(fname);
  except
    info := nil;
  end;
  
  if (info = nil) or info.Exists then
    begin
      repeat
        var dt := DateTime.Now;
        var id := System.Threading.Thread.CurrentThread.ManagedThreadId xor dt.ToBinary();
        result := DateTime.Now.ToString('yyyy-MM-ss_HH.mm.ss') + '_' + id.ToString('x8') + '.hex';
      until not &File.Exists(result);
    end
  else
    result := fname;
end;

begin
  {$region CommandLineArgs}
  var args := Environment.GetCommandLineArgs();
  
  var port    := '';
  var source  := '';
  var block   := 32;
  var flash   := 4096;
  var start   := $F00;
  var backup  := '';
  var baud    := 9600;
  var timeout := 5000;
  var retries := 5;
  var reduce  := true;
  var verbose := false;
  var help    := args.Length = 1;
  
  var OptionRegex := new Regex('-(?<opt>[pfbcskdtrnv?])(=(?<val>.+))?', RegexOptions.IgnoreCase);
  
  foreach var arg in args do
    begin
      var option := OptionRegex.Match(arg);
      
      if option.Success then
        begin
          var s := option.Groups.Count > 2 ? option.Groups[3].Value : '';
          var i := ToInteger(s);
          
          case option.Groups[2].Value.ToLower() of
            'p': port    := s;
            'f': source  := s;
            'b': block   := i;
            'c': flash   := i;
            's': start   := i;
            'k': backup  := s;
            'd': baud    := i;
            't': timeout := i;
            'r': retries := i;
            'n': reduce  := false;
            'v': verbose := true;
            '?': help    := true;
          end;
        end;
    end;
  
  if help then
    Console.WriteLine
    (
      '-p=<port>     [portname] Specify COM port.'#13#10 +
      '-f=<firmware> [filename] Specify the name of the file in ItelHex format containing the firmware.'#13#10 +
      '-b=<block>    [int|hex]  Specify the data block size of the target microcontroller'#39's flash memory.'#13#10 +
      '-c=<flash>    [int|hex]  Specify the flash memory size of the target microcontroller.'#13#10 +
      '-s=<start>    [int|hex]  Specify the start of the bootloader code (lower limit of user code).'#13#10 +
      '-k=<backup>   [filename] Make a backup before downloading the firmware.'#13#10+
      '-d=<baud>     [int]      Specify the baudrate.'#13#10 +
      '-t=<timeout>  [50-10000] Specify the wait command response maximum timeout, ms.'#13#10 +
      '-r=<retries>  [0-10]     Specify the maximum number of retries when commands fail.'#13#10 +
      '-n  noreduce             Overrides the value that specifies whether to truncate empty data in the output IntelHex file.'#13#10 +
      '-v  verbose              Overrides the value that specifies verbose output of diagnostic messages when loading firmware.'#13#10 +
      'Defaults:'#13#10 +
      '    block  : 32 words'#13#10 +
      '    flash  : 4k words'#13#10 +
      '    start  : 0x0F00'#13#10 +
      '    backup : no'#13#10+
      '    baud   : 9600'#13#10+
      '    timeout: 2000'#13#10+
      '    retries: 5'#13#10+
      '    reduce : yes'#13#10+
      '    verbose: no'#13#10
    );
  
  if args.Length = 1 then
    Environment.Exit(0);
  {$endregion}
  
  {$region Check}
  if SerialPort.GetPortNames().FindIndex(p -> p = port) = -1 then
    ErrorExit($'The specified com port "{port}" is missing.');
  
  if block = -1 then
    ErrorExit($'The specified block size is incorrect.');
  
  if flash = -1 then
    ErrorExit($'The specified flash size is incorrect.');
  
  if (start = -1) or (start >= flash) then
    ErrorExit($'The specified boot start is incorrect.');
  
  if baud = -1 then
    ErrorExit($'The specified baurate is incorrect.');
  
  if (timeout < 50) or (timeout > 10000) then
    ErrorExit($'The specified timeout is incorrect.');
  
  if (retries < 0) or (retries > 10) then
    ErrorExit($'The specified max retries is incorrect.');
  {$endregion}
  
  {$region MainTask}
  var loader: BootloaderConsole;
  try
    loader           := new BootloaderConsole(port, baud, retries, reduce, verbose);
    loader.FlashSize := flash;
    loader.BootStart := start;
    loader.BlockSize := block;
    loader.Timeout   := timeout;
  except on ex: Exception do
    ErrorExit($'Initialization error: {ex.Message}{#13#10}{ex.StackTrace}');
  end;
  
  if loader.Open() then
    begin
      Console.WriteLine($'Port "{port}" opened.');
      
      // Check presence bootloader firmware
      var next := loader.IsPresence();
      
      if next then
        Console.WriteLine('Bootloader firmware is OK.')
      else
        Console.WriteLine('Bootloader firmware not responding.');
      
      // Backup
      if next * -String.IsNullOrEmpty(backup) then
        begin
          if not backup.EndsWith('.hex') then
            backup += '.hex';
          var fname := VerifyFileName(backup);
          if fname <> backup then
            begin
              Console.WriteLine($'The given file name "{backup}" is incorrect or such a file already exists.');
              Console.WriteLine($'So the file name was changed to "{fname}".');
            end;
            
          try
            loader.FirmwareDownload(fname);
            Console.WriteLine($'Backup saved to file "{fname}".');
          except 
            on ex: BootloaderException do
              begin
                Console.WriteLine();
                Console.WriteLine($'FirmwareDownload error: {ex.Message}');
                next := false;
              end;
            on ex: Exception do
              begin
                Console.WriteLine();
                Console.WriteLine($'FirmwareDownload error: {ex.Message}');
                Console.WriteLine(ex.StackTrace);
                next := false;
              end;
          end;
        end;
      
      // Load firmware
      next *= -String.IsNullOrEmpty(source);
      if next then
        begin
          next := &File.Exists(source);
          
          if not next then
            Console.WriteLine($'The specified firmware file "{source}" is missing.');
        end;
      
      if next then
        begin
          try
            loader.FirmwareUpload(source);
            Console.WriteLine($'Firmware from file "{source}" loading completed.');
          except 
            on ex: BootloaderException do
              begin
                Console.WriteLine();
                Console.WriteLine($'FirmwareDownload error: {ex.Message}');
                next := false;
              end;
            on ex: Exception do
              begin
                Console.WriteLine();
                Console.WriteLine($'FirmwareUpload error: {ex.Message}');
                Console.WriteLine(ex.StackTrace);
                next := false;
              end;
          end;
        end;
      
      loader.Close();
      Console.WriteLine($'Port "{port}" closed.');
      Console.WriteLine('Done.');
    end
  else
    Console.WriteLine($'Open port "{port}" fail.');
  
  loader.Dispose();
  {$endregion}
end.