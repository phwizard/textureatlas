#include "mainwindow.h"

#include <QApplication>
 #include <QMenuBar>
 #include <QGroupBox>
 #include <QGridLayout>
 #include <QSlider>
 #include <QLabel>
 #include <QTimer>

MainWindow::MainWindow()
{
	ui.setupUi(this);

	//ui.comboBoxResolution

	ui.comboBoxResolution->addItem("2048*2048", 2048);
	ui.comboBoxResolution->addItem("1024*1024", 1024);
	ui.comboBoxResolution->addItem("512*512", 512);
	ui.comboBoxResolution->addItem("256*256", 256);
	ui.comboBoxResolution->addItem("128*128", 128);

	QAction *actionSave = new QAction(QIcon(""), tr("&Save"), this);
	actionSave->setShortcut(QKeySequence::Save);
	connect(actionSave, SIGNAL(triggered()), this, SLOT(saveFile()));
	actionSave->setEnabled(true);
	ui.menubar->addAction(actionSave);


	ui.listViewTextures->setSpacing(10);
	//ui.listViewTextures->setMovement(QListView::Static);
	ui.listViewTextures->setUniformItemSizes(true);
	//ui.listViewTextures->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.listViewTextures->setSelectionRectVisible(true);
	ui.listViewTextures->setFlow(QListView::LeftToRight);
	ui.listViewTextures->setLayoutMode(QListView::Batched);
	//ui.listViewTextures->setResizeMode(QListView::Adjust);
	ui.listViewTextures->setViewMode(QListView::IconMode);
	ui.listViewTextures->setIconSize(QSize(10, 10));

	ui.listViewTextures->setDragEnabled(true);
	ui.listViewTextures->setAcceptDrops(true);
	ui.listViewTextures->setDropIndicatorShown(true);


	textureModel = new TextureModel(this);
	//textureModel->addTexture("red.png");
	//textureModel->addTexture("orange.png");

	ui.listViewTextures->setModel(textureModel);

	ui.workArea->setAcceptDrops(true);
	ui.workArea->setTextureModel(textureModel);

	//this->setBackgroundRole(QPalette::ToolTipBase);
	//this->resize(800, 600);

	//ui.workArea->setStatusBar(ui.statusbar);

	connect(ui.toolButtonMakeAtlas,SIGNAL(clicked(bool)), textureModel,SLOT(arrangeImages()));

	connect(ui.pushButtonLoadFile,SIGNAL(clicked(bool)), this,SLOT(loadFile()));
	connect(ui.pushButtonSaveFile,SIGNAL(clicked(bool)), this,SLOT(saveFileAs()));

	connect(ui.toolButtonAddFile,SIGNAL(clicked(bool)), this,SLOT(AddFile()));
	connect(ui.toolButtonAddFolder,SIGNAL(clicked(bool)), this,SLOT(AddFolder()));
	connect(ui.toolButtonClear,SIGNAL(clicked(bool)), textureModel,SLOT(clear()));

	connect(ui.comboBoxResolution,SIGNAL(currentIndexChanged(int)), this,SLOT(resolutionAtlasChange()));
	connect(ui.toolButtonAddResolution,SIGNAL(clicked(bool)), this,SLOT(AddNewResolution()));

	connect(ui.toolButtonBinding,SIGNAL(clicked(bool)), ui.workArea,SLOT(setBinding(bool)));
	ui.workArea->setBinding(ui.toolButtonBinding->isChecked());


	//connect(ui.toolButtonStartStopSim,SIGNAL(clicked(bool)), ui.workArea,SLOT(startStopSimulation(bool)));

	ui.comboBoxResolution->setCurrentIndex(1);
	resolutionAtlasChange();
}

MainWindow::~MainWindow()
{
}

void MainWindow::resolutionAtlasChange()
{
	int w= ui.comboBoxResolution->itemData(ui.comboBoxResolution->currentIndex()).toInt();
	textureModel->setAtlasSize(w,w);
}

void MainWindow::setCurrentFileName(const QString &fileName)
{
	this->fullFileName = fileName;

	QString shownName;
	if (fullFileName.isEmpty())
		shownName = "untitled.xml";
	else
		shownName = QFileInfo(fullFileName).fileName();

	setWindowTitle(shownName);
	setWindowModified(false);
}




void MainWindow::loadFile()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Load Atlas..."),
					QString(), tr("Atlas files (*.png);;All Files (*)"));

	if ((!path.isEmpty()) && (QFile::exists(path)))
	{
		textureModel->LoadAtlas(path);
		/*
		QMessageBox::warning(this, tr("WYSIWYG Editor"),
					tr("Error loading level\n"),
					QMessageBox::Ok);//, QMessageBox::Ok);
		*/
	}
}

bool MainWindow::saveFileAs()
{
	QString fn = QFileDialog::getSaveFileName(this, tr("Save Atlas"),
					"",	QString());
	if (fn.isEmpty())
		return false;

	//if (! (fn.endsWith(".xml", Qt::CaseInsensitive) ))
	//	fn += ".xml";
	setCurrentFileName(fn);

	return saveFile();
}

bool MainWindow::saveFile()
{
	if (fullFileName.isEmpty())
		return saveFileAs();

	textureModel->SaveAtlas(fullFileName);
	/*
	QFileInfo fi(fullFileName);
	//if (fi.exists())
	{
		qDebug() << fi.path();
		//fi.fileName()
		QString dir = fi.path();

		if (dir.at(dir.length()-1) !=  QDir::separator())
			dir.append(QDir::separator());
		ui.statusbar->showMessage("saving file...");
		if (ui.workArea->gameWorld.saveXmlFile(dir.toStdString(),fi.fileName().toStdString()))
		{
			setWindowModified(false);
			ui.statusbar->showMessage("saved");
			return true;
		}
		else
		{
			QMessageBox::warning(this, tr("WYSIWYG Editor"),
					tr("Error saving level\n"),
				QMessageBox::Ok);//, QMessageBox::Ok);
			ui.statusbar->showMessage("Error saving level");
			return false;
		}

	}

	ui.workArea->updateGL();
	*/
	return true;
}


/*
void MainWindow::SetWorldTexture()
{
	QString path = QFileDialog::getOpenFileName(this,
			tr("Open Image"), "", tr("Image Files (*.png)"));
	if (!path.isEmpty())
	{
		ui.workArea->gameWorld.texNum = textureModel->addTexture(path.toStdString());


		textureModel->recalculateResultImage();
		//ui.workArea->gameWorld.resultTextureId = textureModel->resultTextureId;
		ui.workArea->updateGL();
	}
}
*/

void MainWindow::AddFile()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Load Level..."),
					QString(), tr("PNG file (*.png);;All Files (*)"));

	if ((!path.isEmpty()) && (QFile::exists(path)))
		textureModel->addTexture(path);
}

void MainWindow::AddFolder()
{
	QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open image folder"),
													 "",
													 QFileDialog::ShowDirsOnly
													 | QFileDialog::DontResolveSymlinks);

	if (!dirPath.isEmpty())
	{
		QDir dir;
		dir.setPath(dirPath);

		dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		QStringList filters;
		filters << "*.png" << "*.PNG";
		dir.setNameFilters(filters);

		QStringList listFiles = dir.entryList();

		for (int i = 0; i < listFiles.size(); ++i)
		{
			textureModel->addTexture(dirPath+dir.separator()+listFiles.at(i), false);
		}
		textureModel->arrangeImages();
	}
}

void MainWindow::AddNewResolution()
{
	bool ok;
	int i = QInputDialog::getInteger(this, tr("Add new atlas resolution"),
									 tr("Resolution:"), 1024, 1, 100000, 1, &ok);
	if (ok)
	{
		QString s = QString::number(i);
		s = s+"*"+s;
		ui.comboBoxResolution->addItem(s, i);
	}
}
