#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QTreeWidget>
#include <QStatusBar>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressBar>
#include <QToolBar>
#include <QToolButton>
#include <QComboBox>


#include "ui_mainform.h"

#include "texturemodel.h"
//#include "atlasthread.h"
#include "texturelistwidget.h"


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

public slots:
	void loadFile();
	bool save();
	bool saveAs();
	bool saveFile(QString _fullPath);

	void AddFile();
	void AddFolder();

	void CantMakeAtlas();

private:
	void setCurrentFileName(const QString &fileName);

private slots:
	void resolutionAtlasChange();/// change atlas size
	void AddNewResolution();

	void processStarted();
	void processEnded();

private:
	Ui::MainWindow ui;
	TextureModel *textureModel;

	QString curFileName;

	//AtlasThread *atlasThread;
	QLabel *processLabel;/// label, showed when thread make atlas
	QWidget *proccesWidget;
	QProgressBar *progressBar;

	QComboBox *comboBoxResolution;
	QString lastDir;
};

#endif // MAINWINDOW_H
