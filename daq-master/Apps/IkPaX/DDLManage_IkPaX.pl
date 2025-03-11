#!/usr/bin/perl

# DDLs are a lot more limited now.
# ONLY the IkPmMessage include will be used!!!!

sub varheaderprint {
  my ($number, $name, $vector, $type) = @_;
  print CPP "  // MEMBER $number\n";
  print CPP "  // NAME ".$name."\n";
  print CPP "  // TYPE ";
  if ($vector == 1)
    {
      print CPP "VECTOR ";
    }
  print CPP $type."\n";
}

my @listing = `ls *.ddl`;
my @IKClist;
my $numclasses = 0;
my $item;
foreach $item(@listing)
  {
    my $line; # current line in file
    my $classname; # class name of file
    my $startclass = 0; # whether we've passed the \{ or not
    my $numvars = 0;
    my @filelist = `cat $item`; # list of lines in the file
    my @memberlist;
    my @typearr;

    foreach $line(@filelist)
      {
	if ($line =~ m/class \w* \{/i)
	  {
	    $classname = $line;
	    $classname =~ s/class//;
	    $classname =~ s/\{//;
	    $classname =~ s/^\s*//;
	    $classname =~ s/\n//;
	    $classname =~ s/\r//;
	    $classname =~ s/\s*$//;
	    print "Class: " . $classname . "\n";
	  }
	if ($startclass == 1)
	  {
	    if ($line =~ m/\}/)
	      {
		$startclass = 0;
	      }
	    else
	      {
		$line =~ s/\/\*.*\*\///;
		if (!($line =~ m/\S/))
		  {
		    # comment
		  }
		else
		  {
		    $line =~ s/^\s*//;
#		    print "$line\n";
		    @typearr = split(/ /, $line);
		    $typearr[1] =~ s/\n//;
		    $typearr[1] =~ s/\r//;
		    my $vector = 0;
		    for ($typearr[0]) {
		      /vector/ && do
			{
#			  print "VECTOR!!!\n";
			  $vector = 1;
			};
		      /int/ && do
			{
#			  print "INT!!!\n";
			  $memberlist[$numvars]{'type'} = "int";
			  $memberlist[$numvars]{'name'} = $typearr[1];
			  $memberlist[$numvars]{'vector'} = $vector;
			  $numvars++;
			  last;
			};
		      /string/ && do
			{
#			  print "STRING!!!!\n";
			  $memberlist[$numvars]{'type'} = "string";
			  $memberlist[$numvars]{'name'} = $typearr[1];
			  $memberlist[$numvars]{'vector'} = $vector;
			  $numvars++;
			  last;
			};
#		      /vector/ && do {print "VECTOR!!!!\n";};
		      /include/ && do 
			{
# heh. this is the contents of IkPmInclude.inc
#			  print "INCLUDE!!!!\n";
			  $memberlist[$numvars]{'type'} = "string";
			  $memberlist[$numvars]{'name'} = "mode";
			  $memberlist[$numvars]{'vector'} = 0;
			  $numvars++;
			  $memberlist[$numvars]{'type'} = "int";
			  $memberlist[$numvars]{'name'} = "addresses";
			  $memberlist[$numvars]{'vector'} = 1;
			  $numvars++;
			};
		      /char/ && do
			{
#			  print "CHAR!!!!\n";
			  $memberlist[$numvars]{'type'} = "char";
			  $memberlist[$numvars]{'name'} = $typearr[1];
			  $memberlist[$numvars]{'vector'} = $vector;
			  $numvars++;
			  last;
			};
		      /unsigned/ && do
			{
#			  print "UNSIGNED!!!!\n"
			  $memberlist[$numvars]{'type'} = "unsigned";
			  $memberlist[$numvars]{'name'} = $typearr[1];
			  $memberlist[$numvars]{'vector'} = $vector;
			  $numvars++;
			  last;
			};
		      /float/ && do
			{
#			  print "FLOAT!!!!!\n";
			  $memberlist[$numvars]{'type'} = "float";
			  $memberlist[$numvars]{'name'} = $typearr[1];
			  $memberlist[$numvars]{'vector'} = $vector;
			  $numvars++;
			  last;
			};
		    }
		  }
	      }
	  }
	if ($line =~ m/\{/) # open bracket
	  {
	    $startclass = 1;
	  }
	# end parsing
      }
    $IKClist[$numclasses] = $classname;
    $numclasses++;
    print "Going to write file for " . $classname . "\n";
    my $headername = $classname . ".hxx";
    print "Filename is " . $headername . "\n";
    print "number of elements: " . @memberlist . "\n";
    for (my $i=0; $i < $numvars; $i++)
      {
	print $memberlist[$i]{'name'} . " ";
	if ($memberlist[$i]{'vector'} == 1)
	  {
	    print "vector of ";
	  }
	print $memberlist[$i]{'type'} . "\n";
      }

# Generate header file
# open $classname.h
    open (HEADER, ">".$headername) or die ("Couldn't create $headername: $! \n");
# Add exclusion (ifdef)
    my $headerlock = $headername;
    $headerlock =~ tr/[a-z]/[A-Z]/;
    $headerlock =~ s/\./_/;
    $headerlock = "_" . $headerlock;
    print HEADER "#ifndef " . $headerlock . "\n";
    print HEADER "#define " . $headerlock . "\n";
    print HEADER "\n";
# Main header
    print HEADER "#include <IkMessage.h>\n";
    print HEADER "#include <string>\n";
    print HEADER "#include <vector>\n";
    print HEADER "\n";
    print HEADER "class " . $classname . " : public IkMessage\n";
    print HEADER "{\n";
    print HEADER "public:\n";
# Print class members
    for (my $i=0; $i < $numvars; $i++)
      {
	print HEADER "  ";
	if ($memberlist[$i]{'vector'} == 1)
	  {
	    print HEADER "vector<";
	  }
	print HEADER $memberlist[$i]{'type'};
	if ($memberlist[$i]{'vector'} == 1)
	  {
	    print HEADER ">";
	  }
	print HEADER " " . $memberlist[$i]{'name'} . ";\n";
      }
# Print functions
    print HEADER "  string typeName;\n";
    print HEADER "  void init();\n";
    print HEADER "  void parseError(const char *message);\n";
    print HEADER "\n";
# Print constructors
    print HEADER "  " . $classname . "();\n";
    print HEADER "  " . $classname . "(std::string &text);\n";
    print HEADER "  " . $classname . "(IkMessage &mess);\n";
# Print ... uh, other functions
    print HEADER "  std::string to_string();\n";
    print HEADER "  void read(std::string text);\n";
    print HEADER "  int sendAs(const char *sender, const char *dest);\n";
    print HEADER "  int send(const char *dest);\n";
    print HEADER "  int send(string &dest);\n";
    print HEADER "};\n";
    print HEADER "\n";
    print HEADER "#endif\n";
    close HEADER;

# Generate class C++ definitions
    my $cppname = $classname . ".cc";
    open(CPP, ">".$cppname) or die("Couldn't create $cppname: $!\n");
    print CPP "#include <string>\n";
    print CPP "#include <cstdio>\n";
    print CPP "#include <cstdlib>\n";
    print CPP "#include <cstdarg>\n";
    print CPP "#include <sstream>\n";
    print CPP "#include <vector>\n";
    print CPP "#include <iostream>\n";
    print CPP "#include <IkMessage.h>\n";
    print CPP "#include <IkClient.h>\n";
    print CPP "\n";
    print CPP "#include \"".$headername."\"\n";
    print CPP "\n";
# init() function
    print CPP "void ".$classname."::init()\n";
    print CPP "{\n";
    for (my $i=0;$i<$numvars;$i++)
      {
	varheaderprint($i, $memberlist[$i]{'name'}, $memberlist[$i]{'vector'}, $memberlist[$i]{'type'});
	if ($memberlist[$i]{'vector'} == 1)
	  {
	    print CPP "  // INITIAL None\n";
	    last;
	  }
	for ($memberlist[$i]{'type'}) {
	  /string/ && do
	    {
	      print CPP "  // INITIAL \"\"\n";
	      print CPP "  ".$memberlist[$i]{'name'} . " =\"\";\n";
	      last;
	    };
	  /int|float|unsigned|char/ && do
	    {
	      print CPP "  // INITIAL 0\n";
	      print CPP "  ".$memberlist[$i]{'name'} . " = 0;\n";
	      last;
	    }
	  }
      }
    print CPP "  //END MEMBER INITIALIZATION\n";
    print CPP "  typeName = \"".$classname."\";\n";
    print CPP "}\n\n";
# init() is done
# parseError()
    print CPP "void ".$classname."::parseError(const char *message)\n";
    print CPP "{\n";
    print CPP "  cerr << \"Error parsing message of type \";\n";
    print CPP "  cerr << typeName;\n";
    print CPP "  cerr << \" \";\n";
    print CPP "  cerr << message;\n";
    print CPP "  cerr << endl;\n";
    print CPP "}\n\n";
# parseError() is done
# constructors
    print CPP $classname."::".$classname."() : IkMessage() { init();}\n";
    print CPP $classname."::".$classname."(std::string &text) : IkMessage() {read(text);}\n";
    print CPP $classname."::".$classname."(IkMessage &mess) : IkMessage() {read(mess.FullMessage());}\n";
    print CPP "\n";
# constructors are done
# to_string()
    print CPP "std::string ".$classname."::to_string ()\n";
    print CPP "{\n";
    print CPP "  std::stringstream result;\n";
    for (my $i=0;$i<$numvars;$i++)
      {
	# Print the comments
	varheaderprint($i, $memberlist[$i]{'name'}, $memberlist[$i]{'vector'}, $memberlist[$i]{'type'});
	# Print the name of the field
	print CPP "  result << \"".$memberlist[$i]{'name'}."=\";\n";

	# Vector handler...
	$memberlist[$i]{'vector'} && do
	  {
	    print CPP "  result << \"{\";\n";
	    print CPP "  for (unsigned int i=0;i<".$memberlist[$i]{'name'}.".size();i++)\n";
	    print CPP "    {\n   ";
	  };
	# Quote strings
	for ($memberlist[$i]{'type'})
	  {
	    /string/ && print CPP "  result << \"\\\"\";\n";
	  }
	# Print the field...
	print CPP "  result << ".$memberlist[$i]{'name'};
	# Vector handler
	$memberlist[$i]{'vector'} && print CPP "[i]";
	print CPP ";\n";
	# Quote strings
	for ($memberlist[$i]{'type'})
	  {
	    /string/ && print CPP "  result << \"\\\"\";\n";
	  }
	# Prettify vectors
	$memberlist[$i]{'vector'} && print CPP "   ";
	print CPP "  result << \" \";\n";

	# Vector handler;
	$memberlist[$i]{'vector'} && do
	  {
	    print CPP "    }\n";
	    print CPP "  result << \"} \";\n";
	  };
      }
    print CPP "  return result.str();\n";
    print CPP "}\n\n";
#end to_string()


#read() - this is the suck one
    print CPP "void ".$classname."::read (std::string text)\n";
    print CPP "{\n";
    print CPP "  string::size_type sepPos, oldPos, startPos, endPos;\n";
    print CPP "\n";
    my $stringname = $classname."Str";
    print CPP "  // STRINGNAME $stringname\n";
    print CPP "  string $stringname;\n";
    print CPP "\n";
    print CPP "  init();\n";
    print CPP "\n";
    print CPP "  if (!FromBuffer((char *) text.c_str()))\n";
    print CPP "    {\n";
    print CPP "      parseError(\"FromBuffer\");\n";
    print CPP "      return;\n";
    print CPP "    }\n";
    print CPP "  $stringname = Content;\n";
    print CPP "  oldPos = 0;\n";
    for (my $i=0;$i<$numvars;$i++)
      {
# endlist is what marks the end of the line, or the vector list
	my $endlist = "|";
# Print the comments
	varheaderprint($i,$memberlist[$i]{'name'},$memberlist[$i]{'vector'},$memberlist[$i]{'type'});
# Jump over the field name and equals sign
	print CPP "  sepPos = $stringname.find(\"".$memberlist[$i]{'name'}."\", oldPos);\n";
	print CPP "  if (sepPos == string::npos)\n";
	print CPP "    {\n";
	print CPP "      parseError(\"reading ".$memberlist[$i]{'name'}."\");\n";
	print CPP "    }\n";
# Field name
	print CPP "  sepPos += strlen(\"".$memberlist[$i]{'name'}."\");\n";
# Equals sign
	print CPP "  sepPos += strlen(\"=\");\n";
	print CPP "  oldPos = sepPos;\n";

	$memberlist[$i]{'vector'} && do
	  {
	    $endlist = "}";
	    print CPP "  startPos = $stringname.find('{',oldPos);\n";
	    print CPP "  endPos = $stringname.find('}',oldPos);\n";
	    print CPP "  if (startPos == string::npos || endPos == string::npos)\n";
	    print CPP "     {\n";
	    print CPP "       parseError(\"reading ".$memberlist[$i]{'name'}."\");\n";
	    print CPP "     }\n";
	    print CPP "  oldPos = startPos+strlen(\"{\");\n";
	    print CPP "  while (oldPos < endPos)\n";
	    print CPP "  {\n";
	  };
	my $begin;
	my $end;
	for ($memberlist[$i]{'type'})
	  {
	    /int/ && do { $begin = ""; $end = " ";};
	    /float/ && do { $begin = ""; $end = " ";};
	    /string/ && do { $begin = "\\\""; $end = "\\\"";};
	    /unsigned/ && do { $begin = ""; $end = " ";};
	    /char/ && do { $begin = ""; $end = " ";};
	  }
# Search for beginning...
	if ($begin)
	  {
	    print CPP "    sepPos = $stringname.find('$begin',oldPos);\n";
	    print CPP "    if (sepPos == string::npos)\n";
	    print CPP "       {\n";
	    print CPP "        parseError(\"reading ".$memberlist[$i]{'name'}."\");\n";
	    print CPP "        return;\n";
	    print CPP "       }\n";
	    print CPP "    sepPos += strlen(\"$begin\");\n";
	    print CPP "    oldPos = sepPos;\n";
	  }
	# Search for ending...
	if ($end) # ???? if not
	  {
	    print CPP "    sepPos = $stringname.find('$end', oldPos);\n";
	    print CPP "    if (sepPos == string::npos)\n";
	    print CPP "       {\n";
	    print CPP "         if ((sepPos = $stringname.find('$endlist',oldPos)) == string::npos)\n";
	    print CPP "            {\n";
	    print CPP "              parseError(\"reading ".$memberlist[$i]{'name'}."\");\n";
	    print CPP "              return;\n";
	    print CPP "            }\n";
	    print CPP "       }\n";
	  }
	# sepPos is now the start, endPos is now the end
	# {"hello" "goodbye"}
	# startPos is 0, endPos is 18
	# sepPos starts out at 1, is incremented by 1 (2) and assigned to oldPos
	# sepPos then becomes 7, and sepPos-oldPos is 5, the right length
	# woo hoo!
	# sepPos then is incremented to 8, assigned to oldPos,
	# sepPos then becomes 9, incremented to 10, assigned to oldPos
	# sepPos then becomes 17, sepPos-oldPos is 7, the right length
	# sepPos then is incremented to 18, and is now >= endPos, vector
	# is done.
	# {17 18}
	# startPos is 0, endPos is 6
	# no start search: oldPos starts out at 1, sepPos is 3
	# sepPos-oldPos is the right length. sepPos gets incremented by 1,
	# assigned to oldPos (start of new!!)
	# end search fails originally, then succeeds with }
	# This should work
	my $subname = $memberlist[$i]{'name'}."Str";
	print CPP "     string $subname = $stringname.substr(oldPos, sepPos-oldPos);\n";
	!$memberlist[$i]{'vector'} && print CPP "     ".$memberlist[$i]{'name'}." = ";
	$memberlist[$i]{'vector'} && print CPP "     ".$memberlist[$i]{'name'}.".push_back(";
	for ($memberlist[$i]{'type'})
	  {
	    /int/ && print CPP "atoi($subname.c_str())";
	    /float/ && print CPP "atof($subname.c_str())";
	    /string/ && print CPP "$subname";
	    /unsigned/ && print CPP "strtoul($subname.c_str(), (char **) NULL, 10)";
	    /char/ && print CPP "$subname.c_str()[0]";
	  };
	$memberlist[$i]{'vector'} && print CPP ")";
	print CPP ";\n";
	print CPP "    sepPos += strlen(\"$end\");\n";
	print CPP "    oldPos = sepPos;\n";
	$memberlist[$i]{'vector'} && do
	  {
	    print CPP "    }\n";
	    print CPP "  oldPos = endPos+strlen(\"} \");\n";
	  }
      }
# dear god, read is done
    print CPP "}\n";

#sendAs
    print CPP "int ".$classname."::sendAs(const char *sender, const char *dest)\n";
    print CPP "{\n";
    print CPP "  const string text = to_string();\n";
    print CPP "  Sender = sender;\n";
    print CPP "  Dest = dest;\n";
    print CPP "  Type = typeName;\n";
    print CPP "  Content = text;\n";
    print CPP "  IkMessageSend(*((IkMessage *) this));\n";
    print CPP "  return 0;\n";
    print CPP "}\n\n";
#send
    print CPP "int ".$classname."::send(string &dest)\n";
    print CPP "{\n";
    print CPP "    return send(dest.c_str());\n";
    print CPP "}\n";
    print CPP "int ".$classname."::send(const char *dest)\n";
    print CPP "{\n";
    print CPP "  if (gAppName == \"\")\n";
    print CPP "    return sendAs(\"ThisProgramForgotToSetTheirIkName\", dest);\n";
    print CPP "  else\n";
    print CPP "    return sendAs(gAppName, dest);\n";
    print CPP "}\n";
  }
# Generate IkCTypes.h
# FIXME FIXME FIXME FIXME
# This needs to be rewritten to preserve numbers on multiple runs
# For now it just uses a predef hash
# Probably in the future it will read the previous one from
# the existing IkCTypes.h, and if it can't find it, it'll use this.
my @predefs;
$predefs[0] = "IKDEBUG";
$predefs[1] = "IKINFO";
$predefs[2] = "IKWARNING";
$predefs[3] = "IKSEVERE";
$predefs[4] = "IKFATAL";
$predefs[5] = "IKPMMESSAGE";
$predefs[6] = "IKSUDBRELOAD";
$predefs[7] = "IKSUINIT";
$predefs[8] = "IKSUPING";
$predefs[9] = "IKSUPONG";
$predefs[10] = "IKRCCONTROLALL";
$predefs[11] = "IKRCCONTROLDONE";
$predefs[12] = "IKRCRELEASEOUT";
$predefs[13] = "IKT3";
$predefs[14] = "IKCONFIGREADY";
$predefs[15] = "IKLSREBOOT";
$predefs[16] = "IKLSSTART";
$predefs[17] = "IKLSSTOP";
$predefs[18] = "IKLSWAKEUP";
$predefs[19] = "IKSOUND";
$predefs[20] = "IKSEVERECANCEL";
$predefs[21] = "IKSEVEREWATCH";
$predefs[22] = "IKSUKILL";
$predefs[23] = "IKSPEECH";
$predefs[24] = "IKLSDOWNLOADACK";
$predefs[25] = "IKLSDOWNLOADCHECK";
$predefs[26] = "IKLSOS9";
$predefs[27] = "IKDOWNLOADSUMMARY";
$predefs[28] = "IKLSOS9ACK";
$predefs[29] = "IKLSREADY";
$predefs[30] = "IKLSBOOTED";
$predefs[31] = "IKLSCONFACK";
$predefs[32] = "IKLSCONTACK";
$predefs[33] = "IKLSERROR";
$predefs[34] = "IKLSLOG";
$predefs[35] = "IKLSPARAMACK";
$predefs[36] = "IKLSPAUSEACK";
$predefs[37] = "IKLSSTARTACK";
$predefs[38] = "IKLSSTOPACK";
$predefs[39] = "IKLSCALREQ";
$predefs[40] = "IKLSCHANGESTATE";
$predefs[41] = "IKLSCONFIG";
$predefs[42] = "IKLSGPSSET";
$predefs[43] = "IKLSLOADCONF";
$predefs[44] = "IKLSLOGREQ";
$predefs[45] = "IKLSMONREQ";
$predefs[46] = "IKLSSAVECONF";
$predefs[47] = "IKLSCONT";
$predefs[48] = "IKLSPARAM";
$predefs[49] = "IKLSPAUSE";
$predefs[50] = "IKBSRECORD";
$predefs[51] = "IKLSRECORD";
$predefs[52] = "IKSDREQUEST";
$predefs[53] = "IKSDSUMMARY";
$predefs[54] = "IKSDUPDATE";
$predefs[55] = "IKDBCONFIG";
$predefs[56] = "IKCTREQUEST";
$predefs[57] = "IKRESTART";
$predefs[58] = "IKMORBOOKTREE";
$predefs[59] = "IKLSGENMESS";

open (IKCTYPES, ">IkCTypes.h") or die ("Couldn't create IkCTypes.h: $!\n");
print IKCTYPES "#ifndef _IKC_TYPES_H\n";
print IKCTYPES "#define _IKC_TYPES_H\n";
print IKCTYPES "\n";
print IKCTYPES "/* THIS FILE IS AUTOMATICALLY GENERATED */\n";
print IKCTYPES "/* PLEASE READ DDLManage_IkCLite.pl!!!  */\n";
print IKCTYPES "/*                                      */\n";
print IKCTYPES "/* REALLY, IT'S NOT THAT HARD!          */\n";
print IKCTYPES "\n";
print IKCTYPES "#include <string>\n";
print IKCTYPES "#include <IkMessage.h>\n";
print IKCTYPES "\n";
print IKCTYPES "typedef enum {\n";
print IKCTYPES "  FIRSTIKMSGCODE = 0,\n";
my $predefsize = @predefs;
for (my $i=0; $i < $predefsize ; $i++)
  {
    print IKCTYPES "  " . $predefs[$i] . " = " . $i . ",\n";
  }
for (my $i=0; $i < $numclasses ; $i++)
  {
    my $matched = 0;
    my $matchstring = $IKClist[$i];
    $matchstring =~ tr/[a-z]/[A-Z]/;

    for (my $j=0; $j < $predefsize ; $j++)
      {
	if ($matchstring =~ m/$predefs[$j]/)
	  {
	    $matched = 1;
	  }
      }
    if (!$matched)
      {
	print "Adding " . $IKClist[$i] . " as message $numclasses.\n";
	$predefs[$predefsize] = $matchstring;
	print IKCTYPES "  " . $matchstring . " = " . $numclasses . ",\n";
	$predefsize++;
      }
  }
print IKCTYPES "  LASTIKMSGCODE\n";
print IKCTYPES "} IkCodes;\n";
print IKCTYPES "\n";
print IKCTYPES "const std::string IkTypeMap[LASTIKMSGCODE] =\n";
print IKCTYPES "  {\n";
for (my $i=0; $i < $predefsize ; $i++)
  {
    for (my $j=0; $j< $numclasses ; $j++)
      {
	if ($predefs[$i] =~ m/^$IKClist[$j]$/i)
	  {
	    print IKCTYPES "     \"" . $IKClist[$j] . "\"";
	    if ($i < $predefsize-1)
	      {
		print IKCTYPES ",";
	      }
	    print IKCTYPES "\n";
	  }
      }
  }
print IKCTYPES "  };\n";
print IKCTYPES "\n";
print IKCTYPES "const std::string IkUnknownType=\"IkUnknownType\";";
print IKCTYPES "\n";
for (my $i=0; $i < $predefsize ; $i++)
  {
    for (my $j=0; $j< $numclasses ;$j++)
      {
	if ($predefs[$i] =~ m/^$IKClist[$j]$/i)
	  {
	    print IKCTYPES "#include \"" . $IKClist[$j] . ".hxx\"\n";
	  }
      }
  }
print IKCTYPES "\n";
print IKCTYPES "IkMessage *IkCTypify(IkMessage &mess);\n";
print IKCTYPES "int IkFindType(string typeStr);\n";
print IKCTYPES "string IkFindTypeStr(int type);\n";
print IKCTYPES "\n";
print IKCTYPES "#endif\n";

open (IKCTYPIFY, ">IkCTypify.cc") or die ("Couldn't create IkCTypify.cc: $!\n");
print IKCTYPIFY "#include \"IkCTypes.h\"\n";
print IKCTYPIFY "#include <IkMessage.h>\n";
print IKCTYPIFY "\n";
print IKCTYPIFY "IkMessage *IkCTypify(IkMessage &mess)\n";
print IKCTYPIFY "{\n";
print IKCTYPIFY "  IkMessage *typifiedMess;\n";
print IKCTYPIFY "\n";
print IKCTYPIFY "  int type = IkFindType(mess.Type);\n";
print IKCTYPIFY "  switch(type)\n";
print IKCTYPIFY "  {\n";
for (my $i=0;$i<$predefsize;$i++)
  {
    for (my $j=0;$j<$numclasses;$j++)
      {
	if ($predefs[$i] =~ m/^$IKClist[$j]$/i)
	  {
	    print IKCTYPIFY "    case ".$predefs[$i].":\n";
	    print IKCTYPIFY "      typifiedMess = new ".$IKClist[$j]."(mess);\n";
	    print IKCTYPIFY "      return typifiedMess;\n";
	  }
      }
  }
print IKCTYPIFY "    default:\n";
print IKCTYPIFY "      typifiedMess = new IkMessage(mess);\n";
print IKCTYPIFY "      return typifiedMess;\n";
print IKCTYPIFY "    }\n";
print IKCTYPIFY "}\n\n";

print IKCTYPIFY "int IkFindType(string typeStr)\n";
print IKCTYPIFY "{\n";
print IKCTYPIFY "  for (unsigned int i=0;i<LASTIKMSGCODE;i++)\n";
print IKCTYPIFY "   {\n";
print IKCTYPIFY "     if (typeStr == IkTypeMap[i])\n";
print IKCTYPIFY "        return i;\n";
print IKCTYPIFY "   }\n";
print IKCTYPIFY " return -1;\n";
print IKCTYPIFY "}\n\n";

print IKCTYPIFY "string IkFindTypeStr(int type)\n";
print IKCTYPIFY "{\n";
print IKCTYPIFY "  if (type<0 || type >= LASTIKMSGCODE)\n";
print IKCTYPIFY "    return IkUnknownType;\n";
print IKCTYPIFY "  else\n";
print IKCTYPIFY "    return IkTypeMap[type];\n";
print IKCTYPIFY "}\n";
close(IKCTYPIFY);

open (IKCFILES,">IkCFiles.cc");
for (my $i=0;$i<$numclasses;$i++)
  {
    print IKCFILES "#include \"".$IKClist[$i].".cc\"\n";
  }
close (IKCFILES);
