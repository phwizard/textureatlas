#include "atlasthread.h"


AtlasThread::AtlasThread(TextureModel *_textureModel, QObject *parent)
	: QThread(parent)
{
	textureModel=_textureModel;
	restart = false;
	abort = false;
}

AtlasThread::~AtlasThread()
{
	mutex.lock();
	abort = true;
	//condition.wakeOne();
	mutex.unlock();
	wait();
}


void AtlasThread::loadAtlas(QString _path)
{
	QMutexLocker locker(&mutex);
	if (!isRunning())
	{
		path=_path;
		typeAction=TA_LOAD_ATLAS;
		start();
	}

	/*
	else
	{
		restart = true;
		condition.wakeOne();
	}
	*/
}

void AtlasThread::saveAtlas(QString _path)
{
	QMutexLocker locker(&mutex);
	if (!isRunning())
	{
		path=_path;
		typeAction=TA_SAVE_ATLAS;
		start();
	}
}

void AtlasThread::arrangeImages()
{
	QMutexLocker locker(&mutex);
	if (!isRunning())
	{
		typeAction=TA_ARRANGE_IMAGES;
		start();
	}
}

void AtlasThread::addTexture(QString _path)
{
	QMutexLocker locker(&mutex);
	if (!isRunning())
	{
		path=_path;
		typeAction=TA_ADD_TEXTURE;
		start();
	}
}

void AtlasThread::addDir(QString _dirPath)
{
	QMutexLocker locker(&mutex);
	if (!isRunning())
	{
		path=_dirPath;
		typeAction=TA_ADD_DIR;
		start();
	}
}

void AtlasThread::setAtlasSize(int size)
{
	QMutexLocker locker(&mutex);
	if (!isRunning())
	{
		atlasSize=size;
		typeAction=TA_CHANGE_ATLAS_SIZE;
		start();
	}
}

void AtlasThread::run()
{
	//forever
	{
		emit processStarted();
		mutex.lock();
		bool _isMustLoadAtlas =  isMustLoadAtlas;
		QString _path = path;
		mutex.unlock();

		switch(typeAction)
		{
			case TA_LOAD_ATLAS:
				textureModel->LoadAtlas(_path);
				break;
			case TA_SAVE_ATLAS:
				textureModel->SaveAtlas(_path);
				break;
			case TA_ARRANGE_IMAGES:
				textureModel->arrangeImages();
				break;
			case TA_ADD_TEXTURE:
				textureModel->addTexture(_path);
				break;
			case TA_CHANGE_ATLAS_SIZE:
				textureModel->setAtlasSize(atlasSize,atlasSize);
				break;

			case TA_ADD_DIR:
				{
					QDir dir(_path);

					dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
					QStringList filters;
					filters << "*";
					dir.setNameFilters(filters);

					QStringList listFiles = dir.entryList();
					QStringList listPathTextures;

					for (int i = 0; i < listFiles.size(); ++i)
						listPathTextures.push_back(_path+dir.separator()+listFiles.at(i));

					textureModel->addTextures(listPathTextures);
				}
				break;

		}

		//if (!restart)
		emit processEnded();

		/*
		mutex.lock();
		if (!restart)
			condition.wait(&mutex);
		restart = false;
		mutex.unlock();
		*/
	}
}


