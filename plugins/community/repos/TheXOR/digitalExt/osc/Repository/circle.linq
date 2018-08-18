<Query Kind="Program">
  <Reference>&lt;RuntimeDirectory&gt;\System.Globalization.dll</Reference>
  <Reference>&lt;RuntimeDirectory&gt;\System.Linq.dll</Reference>
  <Reference>&lt;RuntimeDirectory&gt;\System.Windows.Forms.dll</Reference>
  <Namespace>System</Namespace>
  <Namespace>System.Diagnostics</Namespace>
  <Namespace>System.Drawing</Namespace>
  <Namespace>System.Globalization</Namespace>
  <Namespace>System.IO</Namespace>
  <Namespace>System.Security.Cryptography</Namespace>
  <Namespace>System.Security.Cryptography.X509Certificates</Namespace>
  <Namespace>System.Text</Namespace>
  <Namespace>System.Windows.Forms</Namespace>
</Query>

void Main()
{
	int x = 340;
	int y = 340;
	double step = 2 * Math.PI / 32;
	double angle = -Math.PI / 2.0;
	using (System.IO.StreamWriter file = new System.IO.StreamWriter(@"r:\prova.jzml"))
	{
		preambolone(file);
		for (int k = 0; k < 32; k++)
		{
			int r = 300;
			double cx = Math.Cos(angle);
			double cy = Math.Sin(angle);
			angle += step;
			int pos_x = (int)Math.Round(x + r * cx);
			int pos_y = (int)Math.Round(y + r * cy);
			addLine(file, pos_x, pos_y, k);
		}
		postambolone(file);
	}
}

void postambolone(StreamWriter  file)
{
	file.WriteLine(@"</WINDOW>
<WINDOW class=""Tab"" text=""Settings"" x=""0"" y=""0"" width=""1008"" height=""684"" id=""1"" state=""0"" group=""0"" font=""tahoma,11,0"" >
</WINDOW>
</WINDOW>
</WINDOW>
</JZML>");
}

void preambolone(StreamWriter  file)
{
	file.WriteLine(@"<JZML>
<PROJECT version=""5340"" width=""1024"" height=""724"" osc_target=""-2"" midi_target=""-2"" kbmouse_target=""-2"" skin=""Pixel""/>
<WINDOW class=""JAZZINTERFACE"" text=""Default"" x=""0"" y=""0"" width=""1024"" height=""724"" state=""1"" group=""0"" font=""tahoma,11,0"" >
<WINDOW class=""Container"" text=""Container"" x=""0"" y=""-8"" width=""1024"" height=""732"" state=""1"" group=""0"" font=""tahoma,10,0"" send=""0"" osc_target=""-2"" midi_target=""-2"" kbmouse_target=""-2"" color=""1596013"" label=""1"" tabbar=""1"" meta=""1"">
<WINDOW class=""Tab"" text=""Sequence"" x=""0"" y=""0"" width=""1008"" height=""684"" id=""2"" state=""1"" group=""0"" font=""tahoma,11,0"" >");
}

void addLine(StreamWriter file, int pos_x, int pos_y, int counter)
{
	int dime = 20;
	string oscname = $"/scene5/Knob{counter+1}";
	string s = $@"<WINDOW class=""Knob"" text=""{counter + 1}"" x=""{pos_x}"" y=""{pos_y}"" width=""{dime}"" height=""{dime}"" state=""5"" group=""0"" font=""tahoma,10,0"" send=""1"" osc_target=""-2"" midi_target=""-2"" kbmouse_target=""-2"" color=""865343,1596013"" cursor=""0"" grid=""0"" grid_steps=""1"" label=""0"" mode=""1"" physic=""1"" precision=""3"" type=""0"" unit="""" value=""0"">
<PARAM name=""x="" value=""0.700000"" send=""25"" osc_target=""0"" osc_trigger=""1"" osc_message=""{oscname}"" midi_target=""-1"" midi_trigger=""1"" midi_message=""0x90,0x90,0,0"" midi_scale=""0,16383"" osc_scale=""0.000000,1.000000"" kbmouse_target=""-1"" kbmouse_trigger=""1"" kbmouse_message=""0,0,0"" kbmouse_scale=""0,1,0,1""/>
<PARAM name=""z="" value=""0.000000"" send=""16"" osc_target=""0"" osc_trigger=""1"" osc_message=""/Knob/z"" midi_target=""-1"" midi_trigger=""1"" midi_message=""0x90,0x90,0,0"" midi_scale=""0,16383"" osc_scale=""0.000000,1.000000"" kbmouse_target=""-1"" kbmouse_trigger=""1"" kbmouse_message=""0,0,0"" kbmouse_scale=""0,1,0,1""/>
<VARIABLE name=""value=x"" send=""0"" osc_target=""0"" osc_trigger=""1"" osc_message=""/Knob/value"" midi_target=""-1"" midi_trigger=""1"" midi_message=""0x90,0x90,0,0"" midi_scale=""0,16383"" kbmouse_target=""-1"" kbmouse_trigger=""1"" kbmouse_message=""0,0,0"" kbmouse_scale=""0,1,0,1""/>
<VARIABLE name=""attraction=1"" send=""0"" osc_target=""0"" osc_trigger=""1"" osc_message=""/Knob/attraction"" midi_target=""-1"" midi_trigger=""1"" midi_message=""0x90,0x90,0,0"" midi_scale=""0,16383"" kbmouse_target=""-1"" kbmouse_trigger=""1"" kbmouse_message=""0,0,0"" kbmouse_scale=""0,1,0,1""/>
<VARIABLE name=""friction=0.9"" send=""0"" osc_target=""0"" osc_trigger=""1"" osc_message=""/Knob/friction"" midi_target=""-1"" midi_trigger=""1"" midi_message=""0x90,0x90,0,0"" midi_scale=""0,16383"" kbmouse_target=""-1"" kbmouse_trigger=""1"" kbmouse_message=""0,0,0"" kbmouse_scale=""0,1,0,1""/>
<VARIABLE name=""speed=1"" send=""0"" osc_target=""0"" osc_trigger=""1"" osc_message=""/Knob/speed"" midi_target=""-1"" midi_trigger=""1"" midi_message=""0x90,0x90,0,0"" midi_scale=""0,16383"" kbmouse_target=""-1"" kbmouse_trigger=""1"" kbmouse_message=""0,0,0"" kbmouse_scale=""0,1,0,1""/>
</WINDOW>";
	
	file.WriteLine(s);

}
