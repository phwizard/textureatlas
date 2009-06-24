#include "texturemodel.h"

TextureModel::TextureModel(QObject *parent):QAbstractItemModel(parent)
{
	atlasWidth = 1024.0;
	atlasHeight = 1024.0;

	resultImage = QImage(QSize(atlasWidth,atlasHeight), QImage::Format_ARGB32_Premultiplied);

	//gameWorldTexVerts=0;
}

TextureModel::~TextureModel()
{
	clear();
}


void TextureModel::clear()
{
	//for (int i=0; i<textures.size(); i++)
		//glWidget->deleteTexture(textures.value(i).textureId);
	textures.clear();
	makeAtlas();
	reset();
}

int TextureModel::addTexture(QString path, bool mustRemakeAtlas)
{
	QFileInfo fi(path);

	//if (fi.exists())
	{
		QImage img;
		img.load(path);

		QString imageNameToAdd;
		imageNameToAdd = fi.fileName();//baseName();

		if (imageNameToAdd.endsWith(".png",Qt::CaseInsensitive))
				imageNameToAdd = imageNameToAdd.left(imageNameToAdd.size()-4);
		imageNameToAdd.replace(QChar(' '),QChar('_'));

		///проверяем-добавляли уже или нет такую текстуру
		for (int i=0; i<textures.size(); i++)
			//if (textures.value(i).img == img)
			if (textures.value(i).name == imageNameToAdd)
				return i;

		{
			beginInsertRows(QModelIndex(), textures.size(), textures.size());

			textures.push_back(TTexture());
			textures.last().img = img;

			textures.last().name = imageNameToAdd;

			//for (int t=0; t<(textures.size()-1); t++)
				//if (textures.value(t).name == textures.last().name)
					//textures.last().name = "_"+textures.last().name;

			textures.last().size = img.width()*img.height();
			textures.last().texNum = textures.size()-1;
			endInsertRows();

			if (mustRemakeAtlas)
				arrangeImages();

			return (textures.size()-1);
		}
	}
	return -1;
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
		//case Qt::SizeHintRole:
		//	return QSize(20,20);
		//	break;
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
	for (int i=0; i<tempTextures.size(); i++)
		if ((!tempTextures[i]->isPacked) &&(tempTextures[i]->img.width() <= S2->w) && ((tempTextures[i]->img.height() <= S2->h)))
		{
			tempTextures[i]->x = S2->x;
			tempTextures[i]->y = S2->y;
			tempTextures[i]->isPacked = true;

			fsRect S3,S4;
			S3 = fsRect(S2->x, S2->y+tempTextures[i]->img.height(),
						tempTextures[i]->img.width(), S2->h - tempTextures[i]->img.height());
			S4 = fsRect(S2->x+tempTextures[i]->img.width(), S2->y,
						S2->w - tempTextures[i]->img.width(), S2->h);
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
	//QPainter painter(&resultImage);
	//painter.setCompositionMode(QPainter::CompositionMode_Source);
	//painter.fillRect(resultImage.rect(), Qt::transparent);


	bool cantMake=false;

	//NodeClass *root = new NodeClass();
	//root->rc = QRectF(0.0,0.0, 1024.0, 1024.0);
	//root->SetRect(0,0,atlasWidth,atlasHeight);

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

	for (int i=0; i<textures.size(); i++)
		for (int j=i; j<textures.size(); j++)
		{
			qSwap(tempTextures[i], tempTextures[j]);

			fsRect S;
			S = fsRect(0,0, atlasWidth, -1);
			totalHeight = 0;

			for (int t=0; t<tempTextures.size(); t++)
			{
				tempTextures[t]->x = 0;
				tempTextures[t]->y = 0;
				tempTextures[t]->isPacked = false;
			}

			bool canMake = true;
			for (int t=0; t<tempTextures.size(); t++)
			{
				if (tempTextures[t]->isPacked)
					continue;

				totalHeight += tempTextures[t]->img.height();

				//if ((tempTextures[t]->img.width() > atlasWidth) || (totalHeight>atlasHeight))
				if (tempTextures[t]->img.width() > atlasWidth)
				{
					canMake=false;
					break;
				}

				tempTextures[t]->x = S.x;
				tempTextures[t]->y = S.y;
				tempTextures[t]->isPacked = true;

				fsRect S2,S1;
				S2 = fsRect(S.x+tempTextures[t]->img.width(), S.y,
							S.w - tempTextures[t]->img.width(), tempTextures[t]->img.height());
				S1 = fsRect(S.x, S.y+tempTextures[t]->img.height(),
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
		}


	for (int t=0; t<textures.size(); t++)
	{
		textures[t].x = optimTex[t].x();
		textures[t].y = optimTex[t].y();
	}

	makeAtlas();

	if ((textures.size()>0) &&(minTotalHeight > atlasHeight))
	//emit atlasTextureUpdated();
		QMessageBox::warning(0, tr("Texture Atlas Maker"),
					tr("Can't make texture atlas\n"), QMessageBox::Ok);

		//


		//NodeClass *curNode=0;
		//if (!(curNode=root->Insert(curTexture)))
		//	cantMake=true;
		//else
		{
			//painter.drawImage(curTexture->x, curTexture->y, curTexture->img);

			/*
			curTexture->texVerts[0] = r.x();
			curTexture->texVerts[1] = atlasHeight - r.y() -curTexture->img.height()+1;

			curTexture->texVerts[2] = r.x();
			curTexture->texVerts[3] = atlasHeight-r.y()-1;

			curTexture->texVerts[4] = r.x()+curTexture->img.width()-1;
			curTexture->texVerts[5] = atlasHeight-r.y()-1;

			curTexture->texVerts[6] = r.x()+curTexture->img.width()-1;
			curTexture->texVerts[7] = atlasHeight - r.y() -curTexture->img.height()+1;
			*/
		}
		//iter.key()
		//iter.value();


	//painter.fillRect(resultImage.rect(), Qt::white);


	/*
	for (int i=0; i<textures.size(); i++)
	{
		for (int v=0; v<8; v++)
		{
			if ((v % 2)==0)
				gameWorldTexVerts[i][v] = textures[i].texVerts[v]/textureWidth;
			else
				gameWorldTexVerts[i][v] = textures[i].texVerts[v]/textureHeight;
		}
	}
	*/	
	//resultImage.save("result.png");
}

void TextureModel::makeAtlas()
{
	QPainter painter(&resultImage);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(resultImage.rect(), Qt::transparent);

	for (int i=0; i<textures.size(); i++)
	{
		painter.drawImage(textures[i].x, textures[i].y, textures[i].img);

			textures[i].texVerts[0] = textures[i].x/(atlasWidth-1);
			textures[i].texVerts[1] = (atlasHeight-1-(textures[i].y+textures[i].img.height()-1))/(atlasHeight-1);

			textures[i].texVerts[2] = textures[i].x/(atlasWidth-1);
			textures[i].texVerts[3] = (atlasHeight-1-textures[i].y)/(atlasHeight-1);

			textures[i].texVerts[4] = (textures[i].x+textures[i].img.width()-1)/(atlasWidth-1);
			textures[i].texVerts[5] = (atlasHeight-1-textures[i].y)/(atlasHeight-1);

			textures[i].texVerts[6] = (textures[i].x+textures[i].img.width()-1)/(atlasWidth-1);
			textures[i].texVerts[7] = (atlasHeight-1-(textures[i].y+textures[i].img.height()-1))/(atlasHeight-1);
	}

	painter.end();

	emit atlasTextureUpdated();
	/*
	for (int i=0; i<textures.size(); i++)
	{
		for (int v=0; v<8; v++)
		{
			if ((v % 2)==0)
				gameWorldTexVerts[i][v] = textures[i].texVerts[v]/textureWidth;
			else
				gameWorldTexVerts[i][v] = textures[i].texVerts[v]/textureHeight;
		}
	}
	*/
}

void TextureModel::LoadAtlas(QString path)
{
	textures.clear();

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
					//QImage img = loadedImage.copy(x,y,w,h);
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
	 /*
	for (int t=0; t<gameWorldTexVerts.size(); t++)
	{
		qDebug ("=-=-");
		TTexture curTex;


		for (int p=0; p<8; p++)
		{
			curTex.texVerts[p] = gameWorldTexVerts[t][p];

			if ((p % 2)==0)
				gameWorldTexVerts[t][p] = gameWorldTexVerts[t][p]/textureWidth;
			else
				gameWorldTexVerts[t][p] = gameWorldTexVerts[t][p]/textureHeight;
		}

		curTex.img = curTex.img.fromImage( resultImage.copy(curTex.texVerts[0], resultSize.height()-curTex.texVerts[3]-1,
									  curTex.texVerts[6]-curTex.texVerts[0],
									  curTex.texVerts[3]-curTex.texVerts[1]));
		//QImage tempim;
		//tempim = resultImage.copy(0,0, 256, 256);
		//curTex.img = curTex.img.fromImage( tempim);

		curTex.size = curTex.img.width()*curTex.img.height();
		curTex.name="test";
		curTex.texNum = t;
		textures.push_back(curTex);
	}
	reset();
	*/
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

 //return s1.toLower() < s2.toLower();
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
	QString headerFName = path;//Р±РµР· ".h"

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
	QFile fileCPP(dir+headerFName+".cpp");
	if ((fileH.open(QIODevice::WriteOnly | QIODevice::Text)) && (fileCPP.open(QIODevice::WriteOnly | QIODevice::Text)))
	{
		QTextStream outH(&fileH);
		QTextStream outCPP(&fileCPP);

		outH << "#ifndef " << headerFName.toUpper() << "_H\n";
		outH << "#define " << headerFName.toUpper() << "_H\n";
		outH << "\n//Created by Texture Atlas Creator @milytia\n";

		outCPP << "#include \"" << headerFName << ".h\"\n";
		outCPP << "\n//Created by Texture Atlas Creator @milytia\n\n";


		outCPP << "\n//{left_bottom, left_top, right_top, right_bottom}\n";

		qSort(textures.begin(), textures.end(), texLessThan);

		for (int i=0; i<textures.size(); i++)
			outH << "#define _" << headerFName << "_" << textures[i].name << "_ " << i <<"\n";

		for (int i=0; i<textures.size(); i++)
		{
			outH << "extern float " << headerFName << "_" << textures[i].name << "[8];\n";
			outCPP << "float " << headerFName << "_" << textures[i].name << "[8] = {";
			for (int p=0; p<7; p++)
				outCPP << textures[i].texVerts[p] << ", ";
			outCPP << textures[i].texVerts[7] << "};\n";
		}

		outH << "\nextern float " << headerFName << "[" << textures.size() << "][8];\n";
		outCPP << "\nfloat " << headerFName << "[" << textures.size() << "][8] = { ";
		for (int i=0; i<textures.size(); i++)
		{
			outCPP << "{";
			for (int p=0; p<7; p++)
				outCPP << textures[i].texVerts[p] << ", ";
			outCPP << textures[i].texVerts[7] << "}";
			if (i < (textures.size()-1))
				outCPP << ",";
			outCPP << "//\t" << textures[i].name << " - " << i << "\n";

		}
		outCPP << "};\n";

		//FIXME:
		outCPP << "//{width,height}\n";
		outH << "\nextern float " << "size_" << headerFName << "[" << textures.size() << "][2];\n";
		outCPP << "\nfloat " << "size_" << headerFName << "[" << textures.size() << "][2] = { ";
		for (int i=0; i<textures.size(); i++)
		{
			outCPP << "{" << textures[i].img.width() << ", "<< textures[i].img.height() << "}";
			if (i < (textures.size()-1))
				outCPP << ",";
			outCPP << "//" << textures[i].name << " - " << i << "\n";
		}
		outCPP << "};\n";

		outH << "#endif\n";

		outH << "////loadformat\n";
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
