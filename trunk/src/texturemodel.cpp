#include "texturemodel.h"

TextureModel::TextureModel(QObject *parent):QAbstractItemModel(parent)
{
	atlasWidth = 1024.0;
	atlasHeight = 1024.0;

	resultImage = QImage(QSize(atlasWidth,atlasHeight), QImage::Format_ARGB32_Premultiplied);
}

TextureModel::~TextureModel()
{
	clear();
}


void TextureModel::clear()
{
	textures.clear();
	makeAtlas();
	reset();
}

int TextureModel::addTexture(QString path, bool mustRemakeAtlas)
{
	QFileInfo fi(path);

	QImage img;
	if (!img.load(path))
		return -1;

	QString imageNameToAdd;
	imageNameToAdd = fi.fileName();//baseName();

	int lastPosPoint = imageNameToAdd.lastIndexOf(QChar('.'));
	if (lastPosPoint != -1)
		imageNameToAdd = imageNameToAdd.left(lastPosPoint);
	imageNameToAdd.replace(QChar(' '),QChar('_'));
	imageNameToAdd.replace(QChar('.'),QChar('_'));

	///check- maybe we added this texture
	for (int i=0; i<textures.size(); i++)
		if (textures.value(i).name == imageNameToAdd)//if (textures.value(i).img == img)//not working correctly
			return i;

	beginInsertRows(QModelIndex(), textures.size(), textures.size());

	textures.push_back(TTexture());
	textures.last().img = img;

	textures.last().name = imageNameToAdd;

	textures.last().size = img.width()*img.height();
	textures.last().texNum = textures.size()-1;
	endInsertRows();

	if (mustRemakeAtlas)
		arrangeImages();

	return (textures.size()-1);

	return -1;
}

int TextureModel::addTextures(QStringList pathList)
{
	for (int i=0; i<pathList.size(); i++)
		addTexture(pathList.at(i), false);
	arrangeImages();
	return 0;
}

QModelIndex TextureModel::index(int row, int column, const QModelIndex &parent) const
{
	if (parent.isValid())
		return QModelIndex();

	if ((column !=0) || (row >= textures.size()))
		return QModelIndex();
	else
	{
		TTexture *t = const_cast<TTexture *>(&textures[row]);
		return createIndex(row, column, t);
	}
	return QModelIndex();
}

QModelIndex TextureModel::parent(const QModelIndex &child) const
{
	return QModelIndex();
}

int TextureModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return  0;
	else
		return textures.size();
}

int TextureModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return  0;
	else
		return textures.size();
}

QVariant TextureModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if ((index.column() != 0) ||(index.row() >= textures.size()))
		return QVariant();

	switch (role)
	{
		case Qt::DisplayRole:
		case Qt::EditRole:
			return textures.value(index.row()).name;
			break;
		case Qt::DecorationRole:
			return textures.value(index.row()).img.scaled(20,20);
			break;
		case Qt::UserRole:
			return textures.value(index.row()).img;
			break;

		case Qt::UserRole+1:
			return textures.value(index.row()).texNum;
			break;
	}
	return QVariant();
}


bool TextureModel::hasChildren(const QModelIndex &parent) const
{
	if (parent.isValid())
		return false;
	else
		return true;
}

Qt::ItemFlags TextureModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
	return (Qt::ItemIsDragEnabled|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
}

void TextureModel::recursivePacking(fsRect *S2)
{
	float dp=1;
	for (int i=0; i<tempTextures.size(); i++)
		if ((!tempTextures[i]->isPacked) &&((tempTextures[i]->img.width()+2*dp) <= S2->w) &&
			(((tempTextures[i]->img.height()+2*dp) <= S2->h)))
		{
			tempTextures[i]->x = S2->x+dp;
			tempTextures[i]->y = S2->y+dp;
			tempTextures[i]->isPacked = true;

			fsRect S3,S4;
			S3 = fsRect(S2->x, S2->y+tempTextures[i]->img.height()+2*dp,
						tempTextures[i]->img.width()+2*dp, S2->h - tempTextures[i]->img.height()-2*dp);
			S4 = fsRect(S2->x+tempTextures[i]->img.width()+2*dp, S2->y,
						S2->w - tempTextures[i]->img.width()-2*dp, S2->h);
			if (S3.w*S3.h > S4.w*S4.h)
			{
				*S2 = S3;
				recursivePacking(S2);
				*S2 = S4;
				recursivePacking(S2);
			}
			else
			{
				*S2 = S4;
				recursivePacking(S2);
				*S2 = S3;
				recursivePacking(S2);
			}
		}
}

void TextureModel::arrangeImages()
{
	//bool cantMake=false;

	QVector <QPoint> optimTex;
	tempTextures.clear();
	for (int t=0; t<textures.size(); t++)
	{
		textures[t].texNum=t;
		tempTextures.push_back(&textures[t]);
		optimTex.push_back(QPoint(0,0));
	}

	float totalHeight = 0;
	float minTotalHeight = 9999999;
	float dp=1;

	int countSteps = 0;
	//FIXME:bad code
	for (int i=0; i<textures.size(); i++)
		for (int j=i; j<textures.size(); j++)
			countSteps++;
	if (countSteps==0)
		countSteps=1;
	int currentStep=0;
	int currentPercent=0;

	emit currentProgress(0);

	for (int i=0; i<textures.size(); i++)
		for (int j=i; j<textures.size(); j++)
		{
			qSwap(tempTextures[i], tempTextures[j]);

			fsRect S;
			S = fsRect(0,0, atlasWidth, -1);
			totalHeight = 0;

			for (int t=0; t<tempTextures.size(); t++)
			{
				tempTextures[t]->x = dp;
				tempTextures[t]->y = dp;
				tempTextures[t]->isPacked = false;
			}

			bool canMake = true;
			for (int t=0; t<tempTextures.size(); t++)
			{
				if (tempTextures[t]->isPacked)
					continue;

				totalHeight += tempTextures[t]->img.height()+2*dp;

				if ((tempTextures[t]->img.width()+2*dp) > atlasWidth)
				{
					canMake=false;
					break;
				}

				tempTextures[t]->x = S.x+dp;
				tempTextures[t]->y = S.y+dp;
				tempTextures[t]->isPacked = true;

				fsRect S2,S1;
				S2 = fsRect(S.x+tempTextures[t]->img.width()+2*dp, S.y,
							S.w - tempTextures[t]->img.width()-2*dp, tempTextures[t]->img.height()+2*dp);
				S1 = fsRect(S.x, S.y+tempTextures[t]->img.height()+2*dp,
							S.w, -1);

				S = S1;
				recursivePacking(&S2);
			}

			if ((canMake) && (totalHeight<minTotalHeight))
			{
				minTotalHeight = totalHeight;
				for (int t=0; t<tempTextures.size(); t++)
				{
					optimTex[tempTextures[t]->texNum].setX(tempTextures[t]->x);
					optimTex[tempTextures[t]->texNum].setY(tempTextures[t]->y);
				}
			}

			currentStep++;
			currentPercent = (currentStep*100)/countSteps;
			emit currentProgress(currentPercent);
		}

	for (int t=0; t<textures.size(); t++)
	{
		textures[t].x = optimTex[t].x();
		textures[t].y = optimTex[t].y();
	}

	makeAtlas();

	if ((textures.size()>0) &&(minTotalHeight > atlasHeight))
		emit cantMakeAtlas();
}

CPoint TextureModel::pixelSpaceToUVSpace(CPoint xy)
{
	return CPoint(xy.x / ((float)atlasWidth), xy.y / ((float)atlasHeight));
}

void TextureModel::pixelCoordToUVCoord(TTexture *texItem)
{
	//12
	//03
	//CPoint lowerLeftUV = pixelSpaceToUVSpace(CPoint(texItem->x, texItem->y));
	CPoint topLeftUV = pixelSpaceToUVSpace(CPoint(texItem->x, texItem->y));
	topLeftUV.y = 1.0f - topLeftUV.y;

	//CPoint UVDimensions = pixelSpaceToUVSpace(CPoint(texItem->img.width(), texItem->img.height()));
	CPoint UVDimensions = pixelSpaceToUVSpace(CPoint(texItem->img.width(), texItem->img.height()));

	CPoint p[4];

	p[0] = CPoint(topLeftUV.x, topLeftUV.y-UVDimensions.y);// Upper-left
	p[1] = topLeftUV;// Lower-left
	p[2] = CPoint(topLeftUV.x+UVDimensions.x, topLeftUV.y);// Upper-right
	p[3] = CPoint(topLeftUV.x+UVDimensions.x, topLeftUV.y-UVDimensions.y);// Lower-left

	for (int np=0, nv=0; np<4; np++, nv+=2)
	{
		texItem->texVerts[nv] = p[np].x;
		texItem->texVerts[nv+1] = p[np].y;
	}
}

void TextureModel::makeAtlas()
{
	QPainter painter(&resultImage);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(resultImage.rect(), Qt::transparent);

	for (int i=0; i<textures.size(); i++)
	{
		painter.drawImage(textures[i].x, textures[i].y, textures[i].img);

		pixelCoordToUVCoord(&textures[i]);
		//TL
		//float lx = textures[i].x/atlasWidth;
		/*
		float lx = (textures[i].x)/atlasWidth;
		float ty = 1.0-(textures[i].y)/atlasHeight;

		//float rx =  (textures[i].x+textures[i].img.width()-1)/(atlasWidth);
		float rx =  (textures[i].x+textures[i].img.width())/(atlasWidth);
		float by = 1-(textures[i].y+textures[i].img.height())/atlasHeight;

			textures[i].texVerts[0] = lx;
			textures[i].texVerts[1] = by;

			textures[i].texVerts[2] = lx;
			textures[i].texVerts[3] = ty;

			textures[i].texVerts[4] = rx;
			textures[i].texVerts[5] = ty;

			textures[i].texVerts[6] = rx;
			textures[i].texVerts[7] = by;
			*/




		/*
			textures[i].texVerts[0] = textures[i].x/(atlasWidth-1);
			textures[i].texVerts[1] = (atlasHeight-1-(textures[i].y+textures[i].img.height()-1))/(atlasHeight-1);

			textures[i].texVerts[2] = textures[i].x/(atlasWidth-1);
			textures[i].texVerts[3] = (atlasHeight-1-textures[i].y)/(atlasHeight-1);

			textures[i].texVerts[4] = (textures[i].x+textures[i].img.width()-1)/(atlasWidth-1);
			textures[i].texVerts[5] = (atlasHeight-1-textures[i].y)/(atlasHeight-1);

			textures[i].texVerts[6] = (textures[i].x+textures[i].img.width()-1)/(atlasWidth-1);
			textures[i].texVerts[7] = (atlasHeight-1-(textures[i].y+textures[i].img.height()-1))/(atlasHeight-1);
			*/
	}

	painter.end();

	emit atlasTextureUpdated();
}

void TextureModel::LoadAtlas(QString path)
{
	textures.clear();
	reset();

	QString headerName;
	headerName = path;
	if (headerName.endsWith(".png",Qt::CaseInsensitive))
				headerName = headerName.left(headerName.size()-4);
	headerName = headerName +".h";

	QFile file(headerName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QImage loadedImage;
	if (!loadedImage.load(path))
		return;

	/// search line that start with "////loadformat", when found-begin load textures data
	bool startProcessLine = false;
	while (!file.atEnd())
	{
		QByteArray line = file.readLine();
		if (startProcessLine)
		{
			QList<QByteArray> l = line.split('=');
			if (l.size()==2)
			{
				QList<QByteArray> rectS = l.at(1).split(',');
				if (rectS.size()==4)
				{
					int x,y,w,h;
					x = rectS[0].toInt();
					y = rectS[1].toInt();
					w = rectS[2].simplified().toInt();
					h = rectS[3].simplified().toInt();
					TTexture newTex;

					newTex.x = x;
					newTex.y = y;
					newTex.img = loadedImage.copy(x,y,w,h);
					newTex.name = QString(l[0].right(l[0].size()-2));
					newTex.size = newTex.img.width() * newTex.img.height();
					newTex.texNum = textures.size()-1;
					textures.push_back(newTex);
				}
			}

		}
		else
			if (line.startsWith("////loadformat"))
				startProcessLine =true;
	}

	setAtlasSize(loadedImage.width(), loadedImage.height());
	reset();
}

struct elemTex
{
	int num;
	TTexture *tex;
	elemTex(int _num, TTexture *_tex){ num=_num; tex=_tex; }
};

bool texLessThan(const TTexture &t1, const TTexture &t2)
{
	QRegExp regExp("\\d+$");
	int pos;
	int num1,num2;
	QString texName1, texName2;

	num1=num2=0;
	pos = t1.name.indexOf(regExp, 0);
	if (pos!=-1)
	{
		texName1 = t1.name.left(pos);
		QString textNum = t1.name.right(t1.name.length()-pos);
		num1 = textNum.toInt();
	}
	else
		texName1= t1.name;

	pos = t2.name.indexOf(regExp, 0);
	if (pos!=-1)
	{
		texName2 = t2.name.left(pos);
		QString textNum = t2.name.right(t2.name.length()-pos);
		num2 = textNum.toInt();
	}
	else
		texName2= t2.name;

	if (texName1==texName2)
	{
		return (num1 < num2);
	}
	else
		return (texName1 < texName2);
}

void TextureModel::SaveAtlas(QString path)
{
	makeAtlas();
	QFileInfo fi(path);

	QString dir = fi.path();
	if (dir.at(dir.length()-1) !=  QDir::separator())
		dir.append(QDir::separator());

	QString fileName = fi.fileName();

	QString imageFullPath;
	QString headerFName = path;

	if (fileName.endsWith(".png",Qt::CaseInsensitive))
		headerFName = fileName.left(fileName.size()-4);
	else
	{
		imageFullPath = path+".png";
		headerFName = fileName;
	}

	resultImage.save(imageFullPath);

	///
	QFile fileH(dir+headerFName+".h");
	if (fileH.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QTextStream outH(&fileH);

		outH << "#ifndef " << headerFName.toUpper() << "_H\n";
		outH << "#define " << headerFName.toUpper() << "_H\n";
		outH << "\n//Created by Texture Atlas Creator @milytia\n";

		outH << "\n//Texture coordinates {left_bottom, left_top, right_top, right_bottom}\n";

		qSort(textures.begin(), textures.end(), texLessThan);

		for (int i=0; i<textures.size(); i++)
			outH << "#define _" << headerFName << "_" << textures[i].name << "_ " << i <<"\n";

		/*
		for (int i=0; i<textures.size(); i++)
		{
			outH << "extern float " << headerFName << "_" << textures[i].name << "[8];\n";
			outCPP << "float " << headerFName << "_" << textures[i].name << "[8] = {";
			for (int p=0; p<7; p++)
				outCPP << textures[i].texVerts[p] << ", ";
			outCPP << textures[i].texVerts[7] << "};\n";
		}
		*/

		outH << "\nstatic const float " << headerFName << "[" << textures.size() << "][8] = { ";
		for (int i=0; i<textures.size(); i++)
		{
			outH << "{";
			for (int p=0; p<7; p++)
				outH << textures[i].texVerts[p] << ", ";
			outH << textures[i].texVerts[7] << "}";
			if (i < (textures.size()-1))
				outH << ",";
			outH << "//\t" << textures[i].name << " - " << i << "\n";

		}
		outH << "};\n\n";

		//FIXME:
		outH << "//{width,height}\n";
		outH << "static const float " << "size_" << headerFName << "[" << textures.size() << "][2] = { ";
		for (int i=0; i<textures.size(); i++)
		{
			outH << "{" << textures[i].img.width() << ", "<< textures[i].img.height() << "}";
			if (i < (textures.size()-1))
				outH << ",";
			outH << "//" << textures[i].name << " - " << i << "\n";
		}
		outH << "};\n\n";

		outH << "///{width/height}\n";
		outH << "\nstatic const float " << "wh_" << headerFName << "[" << textures.size() << "] = { ";
		for (int i=0; i<textures.size(); i++)
		{
			outH << (float)textures[i].img.width()/(float)textures[i].img.height();
			if (i < (textures.size()-1))
				outH << ",";
			outH << "//" << textures[i].name << " - " << i << "\n";
		}
		outH << "};\n";

		///////////
		QString sizeS = "size_" + headerFName;

		outH << "static void drawTexture" << headerFName.toUpper() << "AtPoint(int tex, float x, float y, float z) {" << "\n";
		outH << "   glPushMatrix();" << "\n";
		outH << "   glTexCoordPointer(2, GL_FLOAT, 0, " << headerFName << "[tex]);" << "\n";
		outH << "   glTranslatef(x, y, z);" << "\n";
		outH << "   glScalef(" << sizeS << "[tex][0], " << sizeS << "[tex][1], 1);" << "\n";
		outH << "   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);" << "\n";
		outH << "   glPopMatrix();" << "\n";
		outH << "}" << "\n";
		///////////

		outH << "#endif\n";

		outH << "////loadformat  (x,y,width,height)\n";
		for (int i=0; i<textures.size(); i++)
		{
			outH << "//" << textures[i].name << "=" << textures[i].x << "," << textures[i].y << ","
												<< textures[i].img.width() << "," << textures[i].img.height() << "\n";
		}
		outH << "\0";

		reset();
	}
}

void TextureModel::setAtlasSize(int w, int h)
{
	atlasWidth=w;
	atlasHeight=h;
	resultImage = QImage(QSize(atlasWidth,atlasHeight), QImage::Format_ARGB32_Premultiplied);
	arrangeImages();
}
