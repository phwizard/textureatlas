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

	this->setWindowTitle(tr("Texture Atlas Maker v0.94  (13-09-2010)"));


	QAction *bindingAction = ui.toolBar->addAction(tr("binding"));
	bindingAction->setCheckable(true);
	bindingAction->setChecked(false);
	QAction *remakeAction = ui.toolBar->addAction(tr("remake"));
	QAction *loadAction = ui.toolBar->addAction(QIcon(""), tr("&Open"));
	loadAction->setShortcut(QKeySequence::Open);
	QAction *saveAction = ui.toolBar->addAction(QIcon(""), tr("Save"));
	saveAction->setShortcut(QKeySequence::Save);

	QMenu *fileMenuAct = ui.menubar->addMenu(tr("File"));
	fileMenuAct->addAction(loadAction);
	fileMenuAct->addAction(saveAction);


///////////////////////////////////////////////
	QGridLayout *gridLayout_2 = new QGridLayout(ui.page_textures);



	TextureListWidget *listViewTextures = new TextureListWidget(ui.page_textures);
	listViewTextures->setObjectName(QString::fromUtf8("listViewTextures"));
			QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Expanding);
			sizePolicy2.setHorizontalStretch(0);
			sizePolicy2.setVerticalStretch(0);
			sizePolicy2.setHeightForWidth(listViewTextures->sizePolicy().hasHeightForWidth());
			listViewTextures->setSizePolicy(sizePolicy2);
	gridLayout_2->addWidget(listViewTextures, 0, 0, 1, 4);


	QToolButton *toolButtonAddFile = new QToolButton(ui.page_textures);
	toolButtonAddFile->setText(tr("add file"));
	toolButtonAddFile->setObjectName(QString::fromUtf8("toolButtonAddFile"));

	gridLayout_2->addWidget(toolButtonAddFile, 1, 0, 1, 1);

	QToolButton *toolButtonClear = new QToolButton(ui.page_textures);
	toolButtonClear->setText(tr("clear"));
	toolButtonClear->setObjectName(QString::fromUtf8("toolButtonClear"));
	gridLayout_2->addWidget(toolButtonClear, 1, 2, 1, 1);

	QToolButton *toolButtonAddFolder = new QToolButton(ui.page_textures);
	toolButtonAddFolder->setText(tr("add folder"));
	toolButtonAddFolder->setObjectName(QString::fromUtf8("toolButtonAddFolder"));
	gridLayout_2->addWidget(toolButtonAddFolder, 1, 1, 1, 1);

	QToolButton *toolExport = new QToolButton(ui.page_textures);
	toolExport->setText(tr("export"));
	gridLayout_2->addWidget(toolExport, 1, 3, 1, 1);


	comboBoxResolution = new QComboBox(ui.page_textures);
	comboBoxResolution->setObjectName(QString::fromUtf8("comboBoxResolution"));
	gridLayout_2->addWidget(comboBoxResolution, 2, 0, 1, 2);




	QToolButton *toolButtonAddResolution = new QToolButton(ui.page_textures);
	toolButtonAddResolution->setText(tr("+"));
	toolButtonAddResolution->setObjectName(QString::fromUtf8("toolButtonAddResolution"));
	toolButtonAddResolution->setArrowType(Qt::NoArrow);

	gridLayout_2->addWidget(toolButtonAddResolution, 2, 2, 1, 1);
////////////////////////////////////////



	proccesWidget = new QWidget(this);//, Qt::ToolTip);
	proccesWidget->setStyleSheet("QLabel {"
			 "background-color: green;"
			 "border-style: outset;"
			 "border-width: 2px;"
			 "border-radius: 10px;"
			 "border-color: beige;"
			 "font: bold 14px;"
			 "min-width: 10em;"
			 "padding: 6px;"
			 "color:white;}"
			"QProgressBar::chunk {"
							"background-color: #00CE00;"
							"width: 10px;"
							"margin: 0.5px;};"
			);

	QVBoxLayout *verticalLayout = new QVBoxLayout(proccesWidget);
	processLabel = new QLabel("Wait...\nmaking atlas...", proccesWidget);
	processLabel->setAlignment(Qt::AlignCenter);
	verticalLayout->addWidget(processLabel);

	progressBar = new QProgressBar(proccesWidget);

	progressBar->setValue(0);
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	progressBar->setAlignment(Qt::AlignCenter);
	progressBar->setTextVisible(true);
	progressBar->setInvertedAppearance(false);
	progressBar->setTextDirection(QProgressBar::TopToBottom);

	verticalLayout->addWidget(progressBar);

	//proccesWidget->resize(100,100);
	proccesWidget->adjustSize();
	proccesWidget->hide();
	/////////////////


	comboBoxResolution->addItem("2048*2048", 2048);
	comboBoxResolution->addItem("1024*1024", 1024);
	comboBoxResolution->addItem("512*512", 512);
	comboBoxResolution->addItem("256*256", 256);
	comboBoxResolution->addItem("128*128", 128);

	textureModel = new TextureModel(this);

	listViewTextures->setModel(textureModel);

	ui.workArea->setAcceptDrops(true);
	ui.workArea->setTextureModel(textureModel);
	ui.workArea->setUpdatesEnabled(true);
	ui.workArea->update();

	connect(remakeAction,SIGNAL(triggered(bool)), textureModel,SLOT(arrangeImages()));
	connect(loadAction,SIGNAL(triggered(bool)), this,SLOT(loadFile()));
	connect(saveAction,SIGNAL(triggered(bool)), this,SLOT(saveFileAs()));

	connect(toolButtonAddFile,SIGNAL(clicked(bool)), this,SLOT(AddFile()));
	connect(toolButtonAddFolder,SIGNAL(clicked(bool)), this,SLOT(AddFolder()));
	connect(toolButtonClear,SIGNAL(clicked(bool)), textureModel,SLOT(clear()));
	connect(toolExport,SIGNAL(clicked(bool)), listViewTextures,SLOT(saveSelectedImages()));


	connect(comboBoxResolution,SIGNAL(currentIndexChanged(int)), this,SLOT(resolutionAtlasChange()));
	connect(toolButtonAddResolution,SIGNAL(clicked(bool)), this,SLOT(AddNewResolution()));

	connect(bindingAction, SIGNAL(triggered(bool)),
					ui.workArea,SLOT(setBinding(bool)));

	ui.workArea->setBinding(bindingAction->isChecked());


	connect(textureModel,SIGNAL(cantMakeAtlas()), this,SLOT(CantMakeAtlas()));

	comboBoxResolution->setCurrentIndex(1);
	resolutionAtlasChange();

	/*
	connect(atlasThread,SIGNAL(processStarted()), this,SLOT(processStarted()));
	connect(atlasThread,SIGNAL(processEnded()), this,SLOT(processEnded()));
	*/


	connect(textureModel,SIGNAL(currentProgress(int)), progressBar,SLOT(setValue(int)));
}

MainWindow::~MainWindow()
{
	//delete proccesWidget;
}

void MainWindow::resolutionAtlasChange()
{
	int w= comboBoxResolution->itemData(comboBoxResolution->currentIndex()).toInt();
	ui.workArea->setUpdatesEnabled(false);
	ui.workArea->textureDeleted();
	textureModel->setAtlasSize(w,w);
	ui.workArea->setUpdatesEnabled(true);
}

void MainWindow::setCurrentFileName(const QString &fileName)
{
	this->fullFileName = fileName;

	QString shownName;
	if (fullFileName.isEmpty())
		shownName = "untitled";
	else
		shownName = QFileInfo(fullFileName).fileName();

	setWindowTitle(shownName);
	setWindowModified(false);
}

void MainWindow::loadFile()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Load Atlas..."),
					lastDir, tr("Atlas files (*.png);;All Files (*)"));

	if ((!path.isEmpty()) && (QFile::exists(path)))
	{
		QFileInfo fi(path);
		QString dir = fi.path();
		if (dir.at(dir.length()-1) !=  QDir::separator())
			dir.append(QDir::separator());
		lastDir = dir;

		ui.workArea->setUpdatesEnabled(false);
		ui.workArea->textureDeleted();
		textureModel->LoadAtlas(path);
		ui.workArea->setUpdatesEnabled(true);
		ui.workArea->update();
	}
}

bool MainWindow::saveFileAs()
{
	QString fn = QFileDialog::getSaveFileName(this, tr("Save Atlas"),
					lastDir,	QString());
	if (fn.isEmpty())
		return false;
	setCurrentFileName(fn);
	return saveFile();
}

bool MainWindow::saveFile()
{
	if (fullFileName.isEmpty())
		return saveFileAs();


	QFileInfo fi(fullFileName);
	QString dir = fi.path();
	if (dir.at(dir.length()-1) !=  QDir::separator())
		dir.append(QDir::separator());
	lastDir = dir;

	ui.workArea->setUpdatesEnabled(false);
	textureModel->SaveAtlas(fullFileName);
	ui.workArea->setUpdatesEnabled(true);
	ui.workArea->update();
	return true;
}

void MainWindow::AddFile()
{
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Add files..."),
					lastDir,
					tr("Image Files (*.bmp *.jpg *jpeg *png *tiff);; PNG file (*.png);; JPG file (*.jpg *jpeg);; BMP file (*.bmp);; All Files (*)"));

	QStringList list = files;
	if (list.size()>0)
	{
		QFileInfo fi(list.first());
		QString dir = fi.path();
		if (dir.at(dir.length()-1) !=  QDir::separator())
			dir.append(QDir::separator());
		lastDir = dir;

		ui.workArea->setUpdatesEnabled(false);
		textureModel->addTextures(list);
		ui.workArea->setUpdatesEnabled(true);
		ui.workArea->update();
	}
}

void MainWindow::AddFolder()
{
	QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open image folder"),
													 lastDir,
													 QFileDialog::ShowDirsOnly
													 | QFileDialog::DontResolveSymlinks);
	if (!dirPath.isEmpty())
	{
		lastDir=dirPath;
		ui.workArea->setUpdatesEnabled(false);
		textureModel->addDir(dirPath);
		ui.workArea->setUpdatesEnabled(true);
		ui.workArea->update();
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
		comboBoxResolution->addItem(s, i);
	}
}

void MainWindow::processStarted()
{
	/*
	ui.workArea->setUpdatesEnabled(false);
	progressBar->setValue(0);
	proccesWidget->show();
	*/
	//proccesWidget->move(this->pos()+QPoint(this->width()/2-proccesWidget->width()/2,this->height()/2-proccesWidget->height()/2));
}

void MainWindow::processEnded()
{
	/*
	proccesWidget->hide();
	ui.workArea->setUpdatesEnabled(true);
	*/
}

void MainWindow::CantMakeAtlas()
{
	QMessageBox::warning(0, tr("Texture Atlas Maker"),
					tr("Can't make texture atlas\n"), QMessageBox::Ok);
}

