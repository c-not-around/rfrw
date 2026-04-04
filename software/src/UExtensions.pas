unit UExtensions;


{$reference System.Drawing.dll}
{$reference System.Windows.Forms.dll}


uses System;
uses System.Drawing;
uses System.Windows.Forms;


type
  Resources = static class
    public static function Icon(name: string) := new System.Drawing.Icon(System.Reflection.Assembly.GetEntryAssembly().GetManifestResourceStream(name));
    public static function Image(name: string) := System.Drawing.Image.FromStream(System.Reflection.Assembly.GetEntryAssembly().GetManifestResourceStream(name));
    public static function Bitmap(name: string) := new System.Drawing.Bitmap(System.Reflection.Assembly.GetEntryAssembly().GetManifestResourceStream(name));
  end;
  
  Message = static class
    public static procedure Info(text: string) := MessageBox.Show(text, 'Info', MessageBoxButtons.OK, MessageBoxIcon.Information);
    public static procedure Warning(text: string) := MessageBox.Show(text, 'Warning', MessageBoxButtons.OK, MessageBoxIcon.Warning);
    public static procedure Error(text: string) := MessageBox.Show(text, 'Error', MessageBoxButtons.OK, MessageBoxIcon.Error);
  end;


end.