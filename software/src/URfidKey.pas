unit URfidKey;


uses System;


type
  RfidKeyType = ( T55x7, EM4x05 );
  
  RfidKey = class
    {$region Constants}
    public static RFID_KEY_LENGTH := 5;
    {$endregion}
    
    {$region Fields}
    private _Vendor: byte;
    private _Number: longword;
    {$endregion}
    
    {$region Ctors}
    public constructor (image: string);
    begin
      _Vendor := Convert.ToByte(image.Substring(0, 2), 16);
      _Number := Convert.ToUInt32(image.Substring(2, 8), 16);
    end;
    
    public constructor (raw: array of byte; index: integer := 0);
    begin
      _Vendor := raw[index];
      _Number := 0;
      
      for var i := 0 to 3 do
        _Number := (_Number shl 8) or raw[1+i];
    end;
    {$endregion}
    
    {$region Properties}
    public property Vendor: byte read _Vendor;
    
    public property Number: longword read _Number;
    {$endregion}
    
    {$region Methods}
    public function ToByteArray(): array of byte;
    begin
      result := new byte[RFID_KEY_LENGTH];
      
      result[0] := _Vendor;
      
      for var i := 0 to 3 do
        result[1+i] := _Number shr (8*(3-i));
    end;
    
    public function ToString() := $'{_Vendor:X2}{_Number:X8}';
    {$endregion}
  end;


end.