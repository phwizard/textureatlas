#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui/QMainWindow>
#include <QTreeWidget>
#include <QStatusBar>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>


#include "ui_mainform.h"

#include "texturemodel.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

public slots:
	void loadFile();
	bool saveFile();
	bool saveFileAs();

	void AddFile();
	void AddFolder();

private:
	void setCurrentFileName(const QString &fileName);

private slots:
	//void SetWorldTexture();
	void resolutionAtlasChange();
	void AddNewResolution();



private:
	Ui::MainWindow ui;
	TextureModel *textureModel;

	QString fullFileName;
};

#endif // MAINWINDOW_H
