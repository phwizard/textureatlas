#include <QApplication>
#include <QTextCodec>

#include "mainwindow.h"



int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(texture);
	QApplication a(argc, argv);

	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForTr(codec);


	QRegExp regExp("\\d+$");
	QString str = "the mini56mum024";
	int pos = str.indexOf(regExp, 0);
	if (pos!=-1)
	{
		QString texName = str.left(pos);
		QString textNum = str.right(str.length()-pos);
		qDebug() << texName;
		qDebug() << textNum;
		int n = textNum.toInt();
		qDebug() << n;
	}

	MainWindow w;
	w.show();
	//w.move(800,200);
	//w.showMaximized();
	return a.exec();
}
