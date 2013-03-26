// JScript source code

var versionFileName = "version.";

var needReversion = true;
//размерности файлов для просмотра кол строк, в маленьком регистре
var exts = new Array("c","h","a51","s79");


var source,	verLog, hexFile;
var ForWriting=2;
var ForReading=1;
var ForAppending=8;

/*

шифр файла на выходе Version.txt
тоже самое в файле на входе Version.ver только каждая запись в своей строке

y.w.x.b some text

y - год начала разработки
w - неделя начала разработки
x - количество недель с начала разработки
b - номер сборки

не идет слежка: 
	за файлами версий version.txt и version.ver
	

*/

/////////////начало

FSO=WScript.CreateObject("Scripting.FileSystemObject");

args = WScript.Arguments;
if(args.Length>0)
{
//	hexFile = args.Item(1);
//	hexFile.lenght = hexFile.lenght-1;
	source = args.Item(0);
	source.lenght = source.lenght-1;
}else
{
	WScript.Echo("no arguments");
	WScript.Quit();
}


if(!FSO.FolderExists(source))
{//если нет папки источника
	WScript.Echo("Source dir ", source, " not found.");
	WScript.Quit();
}


var i=0;
var j;


//newVerFolder = createNewVersionFolder();


sourceFld = FSO.GetFolder(source);
files = new Enumerator(sourceFld.Files);

files.moveFirst();

var strr = new String();
var strr2 = new String();
var strr3 = new String();
var count = 0;
var totalLines=0;
var file;
var linesCnt;
var logFileName = source+"\\"+versionFileName+"log";
if(!FSO.FileExists(logFileName))
{
	f = FSO.CreateTextFile(logFileName);
	f.WriteLine("Version log");
	f.Close();
}
verLog = FSO.OpenTextFile(logFileName,ForAppending,true);
strr2 = versionFileName;
strr2 = strr2.toLocaleLowerCase();
strr3 = WScript.ScriptName;
strr3 = strr3.toLocaleLowerCase();
while(!files.atEnd())
{
	file = files.item();
	strr = file.Name;
	strr = strr.toLocaleLowerCase();
	if(strr != (strr2+"ver") &&	//не смотрим файл version.ver
		strr != (strr2+"txt") &&	//не смотрим файл version.txt
		strr != (strr2+"log") &&	//не смотрим файл version.log
		strr != strr3 && //не смотрим этот скрипт файл
		FSO.GetExtensionName(strr)!="bak" &&
		FSO.GetExtensionName(strr)!="scc" &&	//не смотрим файл соурсейфа
		FSO.GetExtensionName(strr)!="opt")
	{
		linesCnt = calcLines(file);
		totalLines+=linesCnt;
		if(count==0)
		{//первая вставка, значит вставим дату и время
			verLog.WriteLine("\r\n");
			now = new Date();
			verLog.WriteLine(now.toLocaleString());
		}
		count++;
	}
	
	files.moveNext();
}
if(count>0)
{
	writeToLog("Total", totalLines);
}
verLog.Close();

if(count!=0 && needReversion)
{//что то менялось, значит меняем данные версии
	//скопируем старый version.txt в папку версии
	var num1;
	var num2;
	var num3;
	var num4;
	var ververFileName = source+"\\"+versionFileName+"ver";
	var vertxtFileName = source+"\\"+versionFileName+"txt";
	if(!FSO.FileExists(ververFileName))
	{//нет файла версий, создадим его
		dNow2 = new Date();
		dNow = new Date(2000,0,1,0,0,0,0);
		num1 = dNow2.getFullYear();
		dNow.setFullYear(num1);
		num2 = Math.round((dNow2.getTime()-dNow.getTime())/(1000*3600*24*7)-0.5);
		num1-=2000;
		f = FSO.CreateTextFile(ververFileName);
		f.WriteLine(num1);
		f.WriteLine(num2);
		f.WriteLine(0);
		f.WriteLine(0);
		f.WriteLine("");
		f.Close();
		
		f = FSO.CreateTextFile(vertxtFileName,true);
		line = "\"" + num1+"."+num2+"."+"0.0"+" " + "\"";
		f.WriteLine(line);
		f.Close();
	}

	
	fileRD = FSO.OpenTextFile(ververFileName,ForReading);
	fileWR = sourceFld.CreateTextFile(versionFileName+"txt",true);
	if(fileRD!=null && fileWR!=null)
	{
		line1 = fileRD.ReadLine();
		num1 = parseInt(line1,10);
		line2 = fileRD.ReadLine();
		num2 = parseInt(line2,10);
		dBegin = new Date(0);
		dBegin.setFullYear(2000+num1);
		dBegin.setDate(num2*7);
		dNow = new Date();
		//расчет количества недель с момента начала проекта до текущей даты
		dMean = (dNow.getTime()-dBegin.getTime())/(1000*3600*24*7);
		//берем только целую часть недель приведенную к наименьшему
		dMean = Math.round(dMean-0.5);
		line3 = fileRD.ReadLine();
		num3 = parseInt(line3,10);
		line4 = fileRD.ReadLine();
		num4 = parseInt(line4,10);
		lineTxt = fileRD.ReadLine();

		var date = new Date();
		var type = date.getTime();
//		newVerFolder.Name = line1+"_"+line2+"_"+line3+"_"+line4+"_"+lineTxt+"."+type;

		if(num3!=dMean)
		{
			num3 = dMean;
			num4 = 0;
		}
		num4++;	//всегда будет номер сборки не менее 1
		
		line = "\"" + num1+"."+num2+"."+num3+"."+num4+ " " + lineTxt + "\"";
		fileWR.WriteLine(line);
		fileWR.Close();
		fileRD.Close();
		fileWR = sourceFld.CreateTextFile(versionFileName+"ver",true);
		fileWR.WriteLine(num1);
		fileWR.WriteLine(num2);
		fileWR.WriteLine(num3);
		fileWR.WriteLine(num4);
		fileWR.WriteLine(lineTxt);
		fileWR.Close();
	}



	//копирование hex файлов
//	FSO.CopyFile(hexFile,newVerFolder.Path+"\\", true)
//	hexFile = newVerFolder.Path+"\\"+hexFile;
///////////////////////////////////////
/*	sourceFld = FSO.GetFolder(hexFile);
	files = new Enumerator(sourceFld.Files);

	files.moveFirst();

	strr = new String();
	while(!files.atEnd())
	{
		file = files.item();
		strr = FSO.GetExtensionName(file.Name);
		strr = strr.toLocaleLowerCase();
		if(strr == "hex")
		{
			strr = FSO.GetFileName(file.Name);
			if(strr.charAt(0)=="_" && strr.charAt(1)=="_")
				file.Copy(newVerFolder.Path+"\\");//будем копировать
		}
		files.moveNext();
	}
	*/
///////////////////////////////////////


}


/////////////конец





function folderSorting(first, second)
{
	var fType = parseInt(FSO.GetExtensionName(first.Name));
	var sType = parseInt(FSO.GetExtensionName(second.Name));
	return fType-sType;
}


//подсчет количчеств строк в тексте
//не считая пустые
function calcLines(file)
{
	var i;
	var lines=0;
	var ext = new String();
	ext = FSO.GetExtensionName(file.Name);
	ext = ext.toLowerCase();
	for(i=0;i<exts.length;i++)
	{
		if(ext==exts[i])
		{
			stream = file.OpenAsTextStream(ForReading);
			while(!stream.AtEndOfStream)
			{
				rdl = stream.ReadLine();
				if(rdl.length>0)
					lines++;
			}
			break;
		}
	}
	return lines;
}


function writeToLog(name, lines)
{
	var str = new String();
	str2 = "";
	if(lines>0)
		str2 = "lines "+lines;
	verLog.WriteLine(name + " : " + str2);
}





