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

	ui.listViewTextures->setSelectionRectVisible(true);
	ui.listViewTextures->setSpacing(2);
	ui.listViewTextures->setTextElideMode(Qt::ElideRight);
	ui.listViewTextures->setViewMode(QListView::ListMode);
	ui.listViewTextures->setMovement(QListView::Static);
	ui.listViewTextures->setFlow(QListView::TopToBottom);

	ui.listViewTextures->setDragEnabled(true);
	ui.listViewTextures->setAcceptDrops(true);
	ui.listViewTextures->setDropIndicatorShown(true);

	textureModel = new TextureModel(this);

	ui.listViewTextures->setModel(textureModel);

	ui.workArea->setAcceptDrops(true);
	ui.workArea->setTextureModel(textureModel);

	atlasThread = new AtlasThread(textureModel, this);

	connect(ui.toolButtonMakeAtlas,SIGNAL(clicked(bool)), atlasThread,SLOT(arrangeImages()));
	connect(ui.pushButtonLoadFile,SIGNAL(clicked(bool)), this,SLOT(loadFile()));
	connect(ui.pushButtonSaveFile,SIGNAL(clicked(bool)), this,SLOT(saveFileAs()));

	connect(ui.toolButtonAddFile,SIGNAL(clicked(bool)), this,SLOT(AddFile()));
	connect(ui.toolButtonAddFolder,SIGNAL(clicked(bool)), this,SLOT(AddFolder()));
	connect(ui.toolButtonClear,SIGNAL(clicked(bool)), textureModel,SLOT(clear()));

	connect(ui.comboBoxResolution,SIGNAL(currentIndexChanged(int)), this,SLOT(resolutionAtlasChange()));
	connect(ui.toolButtonAddResolution,SIGNAL(clicked(bool)), this,SLOT(AddNewResolution()));

	connect(ui.toolButtonBinding,SIGNAL(clicked(bool)), ui.workArea,SLOT(setBinding(bool)));
	ui.workArea->setBinding(ui.toolButtonBinding->isChecked());


	connect(textureModel,SIGNAL(cantMakeAtlas()), this,SLOT(CantMakeAtlas()));

	ui.comboBoxResolution->setCurrentIndex(1);
	resolutionAtlasChange();

	connect(atlasThread,SIGNAL(processStarted()), this,SLOT(processStarted()));
	connect(atlasThread,SIGNAL(processEnded()), this,SLOT(processEnded()));


	connect(textureModel,SIGNAL(currentProgress(int)), progressBar,SLOT(setValue(int)));
}

MainWindow::~MainWindow()
{
	//delete proccesWidget;
}

void MainWindow::resolutionAtlasChange()
{
	int w= ui.comboBoxResolution->itemData(ui.comboBoxResolution->currentIndex()).toInt();
	ui.workArea->setUpdatesEnabled(false);
	ui.workArea->textureDeleted();
	atlasThread->setAtlasSize(w);
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
		ui.workArea->setUpdatesEnabled(false);
		ui.workArea->textureDeleted();
		atlasThread->loadAtlas(path);
	}
}

bool MainWindow::saveFileAs()
{
	QString fn = QFileDialog::getSaveFileName(this, tr("Save Atlas"),
					"",	QString());
	if (fn.isEmpty())
		return false;
	setCurrentFileName(fn);
	return saveFile();
}

bool MainWindow::saveFile()
{
	if (fullFileName.isEmpty())
		return saveFileAs();

	atlasThread->saveAtlas(fullFileName);
	return true;
}

void MainWindow::AddFile()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Add file..."),
					QString(),
					tr("Image Files (*.bmp *.jpg *jpeg *png *tiff);; PNG file (*.png);; JPG file (*.jpg *jpeg);; BMP file (*.bmp);; All Files (*)"));

	if ((!path.isEmpty()) && (QFile::exists(path)))
	{
		ui.workArea->setUpdatesEnabled(false);
		ui.workArea->textureDeleted();
		atlasThread->addTexture(path);
	}
}

void MainWindow::AddFolder()
{
	QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open image folder"),
													 "",
													 QFileDialog::ShowDirsOnly
													 | QFileDialog::DontResolveSymlinks);
	if (!dirPath.isEmpty())
		atlasThread->addDir(dirPath);
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

void MainWindow::processStarted()
{
	ui.workArea->setUpdatesEnabled(false);
	progressBar->setValue(0);
	proccesWidget->show();
	//proccesWidget->move(this->pos()+QPoint(this->width()/2-proccesWidget->width()/2,this->height()/2-proccesWidget->height()/2));
}

void MainWindow::processEnded()
{
	proccesWidget->hide();
	ui.workArea->setUpdatesEnabled(true);
}

void MainWindow::CantMakeAtlas()
{
	QMessageBox::warning(0, tr("Texture Atlas Maker"),
					tr("Can't make texture atlas\n"), QMessageBox::Ok);
}
