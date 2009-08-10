#ifndef ATLASTHREAD_H
#define ATLASTHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "texturemodel.h"

enum TYPE_ACTION
{
	TA_LOAD_ATLAS=0,
	TA_SAVE_ATLAS,
	TA_ARRANGE_IMAGES,
	TA_ADD_TEXTURE,
	TA_ADD_DIR,
	TA_CHANGE_ATLAS_SIZE
};

class AtlasThread : public QThread
{
	Q_OBJECT

public:
	AtlasThread(TextureModel *_textureModel, QObject *parent = 0);
	~AtlasThread();

	void loadAtlas(QString _path);
	void saveAtlas(QString _path);

	void addTexture(QString _path);
	void addDir(QString _dirPath);
	void setAtlasSize(int size);

signals:
	void processStarted();
	void processEnded();
public slots:
	void arrangeImages();

protected:
	void run();

private:
	QMutex mutex;
	//QWaitCondition condition;
	bool restart;
	bool abort;

	TextureModel *textureModel;

	QString path;
	bool isMustLoadAtlas;
	int typeAction;
	int atlasSize;
};

#endif // ATLASTHREAD_H
