#ifndef TEXTUREMODEL_H
#define TEXTUREMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QPixmap>
#include <QIcon>
#include <QFileInfo>
#include <QPainter>
#include <QDebug>
#include <QDir>
#include <vector>

#include "common.h"

struct fsRect{
	float x,y,w,h;
	fsRect(float _x=0,float _y=0,float _w=0,float _h=0):x(_x),y(_y),w(_w),h(_h) {}
};


class TextureModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TextureModel(QObject *parent=0);
	~TextureModel();


	int addTexture(QString path, bool mustRemakeAtlas=true);
	int addTextures(QStringList pathList);
	void delTexture(int num)
	{
		if ((num>=0) && (num<textures.size()))
		{
			textures.remove(num,1);

			makeAtlas();
			reset();//FXIME:переделать на beginDelete
			emit textureDeleted();
		}
	}

	void delTexture(TTexture *tex)
	{
		for (int i=0; i<textures.size(); i++)
			if (&textures[i] == tex)
			{
				textures.remove(i,1);

				makeAtlas();
				reset();//FXIME:переделать на beginDelete
				emit textureDeleted();
			}
	}

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &child) const;

	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;

	QVariant data(const QModelIndex &index, int role) const;
	bool hasChildren(const QModelIndex &parent) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

	void LoadAtlas(QString path);

	void SaveAtlas(QString path);

	void recursivePacking(fsRect *S2);

signals:
	void atlasTextureUpdated();
	void textureDeleted();
	void cantMakeAtlas();

public slots:
	void clear();
	void arrangeImages();//
	void makeAtlas();
	void setAtlasSize(int w, int h);/// change atlas size
public:
	QImage resultImage;
	QVector <TTexture> textures;
	QVector <TTexture *> tempTextures;

	float atlasWidth;
	float atlasHeight;
};

#endif // TEXTUREMODEL_H
