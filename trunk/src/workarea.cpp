#include "workarea.h"

WorkArea::WorkArea(QWidget *parent):QWidget(parent)
{
	textureModel=0;
	selectedTexture=0;

	setFocusPolicy(Qt::StrongFocus);
}

WorkArea::~WorkArea()
{
}

void WorkArea::setTextureModel(TextureModel *_textureModel)
{
	textureModel = _textureModel;

	connect(textureModel,SIGNAL(atlasTextureUpdated()), this,SLOT(update()));
	connect(textureModel,SIGNAL(textureDeleted()), this,SLOT(textureDeleted()));
}

void WorkArea::drawChessBoard(QPainter *painter)
{
		//int countBoxW = textureModel->atlasWidth/widthBox;
		//int countBoxH = textureModel->atlasHeight/heightBox;
		//int widthBox = 4*textureModel->atlasWidth;
		//int heightBox = 4*textureModel->atlasHeight;
		int widthBox = 32;
		int heightBox = 32;

		int countBoxW = textureModel->atlasWidth/widthBox;
		int countBoxH = textureModel->atlasHeight/heightBox;


		painter->setPen(Qt::NoPen);

		painter->setBrush(Qt::lightGray);

		for (int i=0; i<countBoxW; i++)
			for (int j=0; j<countBoxH; j++)
			{
				if (((i+j) % 2)==0)
					painter->drawRect(i*widthBox, j*heightBox, widthBox,heightBox);
			}

		painter->setBrush(Qt::gray);
		for (int i=0; i<countBoxW; i++)
			for (int j=0; j<countBoxH; j++)
			{
				if (((i+j) % 2)!=0)
					painter->drawRect(i*widthBox, j*heightBox, widthBox,heightBox);
			}
}
void WorkArea::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	painter.fillRect(this->rect(), Qt::white);

	if (textureModel)
	{
		QImage *img = &textureModel->resultImage;
		QRectF target(this->rect());
		QRectF source(img->rect());

		//int widthBox = source.width()/target.width();
		//int heightBox = source.height()/target.height();

		//painter.drawImage(target, textureModel->resultImage, source);

		//painter.setViewport();
		int side = qMin(width(), height());
		painter.scale(side / textureModel->atlasWidth, side / textureModel->atlasHeight);
		//painter.scale(width() / textureModel->atlasWidth, height() / textureModel->atlasHeight);

		//
		drawChessBoard(&painter);
		//
		//painter.setViewport(0, 0, width()/3, height()/3);

		//painter.setPen(Qt::darkGreen);
		painter.setBrush(Qt::NoBrush);

		QPen pen;
		pen.setStyle(Qt::DotLine);
		//pen.setWidth(1);
		pen.setBrush(Qt::green);

		painter.setPen(pen);

		for (int i=0; i<textureModel->textures.size(); i++)
		{
			painter.drawImage(textureModel->textures[i].x, textureModel->textures[i].y, textureModel->textures[i].img);

			painter.drawRect(textureModel->textures[i].x, textureModel->textures[i].y,
							 textureModel->textures[i].img.width(), textureModel->textures[i].img.height());
		}

		if (selectedTexture)
		{
			pen.setBrush(Qt::red);
			painter.setPen(pen);
			painter.drawRect(selectedTexture->x, selectedTexture->y,
							 selectedTexture->img.width(), selectedTexture->img.height());
		}
	}
	//painter.setRenderHint(QPainter::Antialiasing);

	//painter.setPen(QColor(50, 100, 120, 200));
	//painter.setPen(QColor(50, 100, 120, 255));


	/*
for (int i=0; i<balls.size(); ++i)
	{
		painter.setBrush(balls[i].color);
		//painter.setBrush(gradient);
		int m_pointSize = balls[i].r;

		painter.drawEllipse(QRect(balls[i].x - m_pointSize,
						balls[i].y - m_pointSize,
						m_pointSize*2, m_pointSize*2));
	}
	*/
}

void WorkArea::mousePressEvent(QMouseEvent *event)
{
	QPoint pGlobal = event->pos();
	//QPointF curPos;
	//curPos.setX((textureModel->atlasWidth-1.0)*(float)pGlobal.x()/((float)this->width()-1.0));
	//curPos.setY((textureModel->atlasHeight-1.0)*(float)pGlobal.y()/((float)this->height()-1.0));


	int side = qMin(width(), height());
	QPointF p;
	p.setX((textureModel->atlasWidth-1.0)*(float)pGlobal.x()/((float)side-1.0));
	p.setY((textureModel->atlasHeight-1.0)*(float)pGlobal.y()/((float)side-1.0));

	selectedTexture=0;

	for (int i=0; i<textureModel->textures.size(); i++)
	{
		if ((p.x() >= textureModel->textures[i].x) && (p.x() < (textureModel->textures[i].x+textureModel->textures[i].img.width())) &&
			(p.y() >= textureModel->textures[i].y) && (p.y() < (textureModel->textures[i].y+textureModel->textures[i].img.height())))
		{
			selectedTexture = &textureModel->textures[i];
			break;
		}
	}

	localPos = p;

	this->update();
}

struct cp
{
	int x,y;
	cp(int _x=0,int _y=0):x(_x),y(_y){}
};

void WorkArea::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pGlobal = event->pos();
	QPointF curPos;

	int side = qMin(width(), height());
	//curPos.setX((textureModel->atlasWidth-1.0)*(float)pGlobal.x()/((float)this->width()-1.0));
	//curPos.setY((textureModel->atlasHeight-1.0)*(float)pGlobal.y()/((float)this->height()-1.0));
	curPos.setX((textureModel->atlasWidth-1.0)*(float)pGlobal.x()/((float)side-1.0));
	curPos.setY((textureModel->atlasHeight-1.0)*(float)pGlobal.y()/((float)side-1.0));

	int dx,dy;
	dx = curPos.x()-localPos.x();
	dy = curPos.y()-localPos.y();

	if ((event->buttons() & Qt::LeftButton))
	{
		if (selectedTexture)
		{
			selectedTexture->x += dx;
			selectedTexture->y += dy;

			if (binding)
			{
				QVector <cp> selectedPoints;
				QVector <cp> notSelectedPoints;
				///

				float minDistSq=10000;
				cp dp(0,0);

				for (int i=0; i<textureModel->textures.size(); i++)
				{
					TTexture *curTex = &textureModel->textures[i];

					QVector <cp> tempPs;

					/*
					tempPs.push_back(cp(curTex->x, curTex->y));
					tempPs.push_back(cp(curTex->x, curTex->y+curTex->img.height()-1));
					tempPs.push_back(cp(curTex->x+curTex->img.width()-1, curTex->y+curTex->img.height()-1));
					tempPs.push_back(cp(curTex->x+curTex->img.width()-1, curTex->y));
					*/


					/*
					tempPs.push_back(cp(curTex->x, curTex->y));
					tempPs.push_back(cp(curTex->x, curTex->y-curTex->img.height()+1));
					tempPs.push_back(cp(curTex->x+curTex->img.width()-1, curTex->y-curTex->img.height()+1));
					tempPs.push_back(cp(curTex->x+curTex->img.width()-1, curTex->y));
					*/

					if (curTex!=selectedTexture)
					{
						tempPs.push_back(cp(curTex->x, curTex->y));
						tempPs.push_back(cp(curTex->x, curTex->y+curTex->img.height()));
						tempPs.push_back(cp(curTex->x+curTex->img.width(), curTex->y+curTex->img.height()));
						tempPs.push_back(cp(curTex->x+curTex->img.width(), curTex->y));
						notSelectedPoints += tempPs;
					}
					else
					{
						tempPs.push_back(cp(curTex->x-1, curTex->y-1));
						tempPs.push_back(cp(curTex->x-1, curTex->y+curTex->img.height()+1));
						tempPs.push_back(cp(curTex->x+curTex->img.width()+1, curTex->y+curTex->img.height()+1));
						tempPs.push_back(cp(curTex->x+curTex->img.width()+1, curTex->y-1));
						selectedPoints = tempPs;
					}
				}

				cp tempP;
				for (int s=0; s<selectedPoints.size(); s++)
					for (int ns=0; ns<notSelectedPoints.size(); ns++)
					{
						tempP.x = notSelectedPoints[ns].x - selectedPoints[s].x;
						tempP.y = notSelectedPoints[ns].y - selectedPoints[s].y;
						if ((tempP.x*tempP.x+tempP.y*tempP.y) < minDistSq)
						{
							dp = tempP;
							minDistSq = tempP.x*tempP.x+tempP.y*tempP.y;
							qDebug() << s;
							//break;
						}
					}

				float dd= 8*(float)textureModel->atlasWidth/(float)width();

				if (minDistSq <= dd*dd)
				//if (minDistSq <= 9)
				{
					selectedTexture->x += dp.x;
					selectedTexture->y += dp.y;
				}
				//else
				{
					//selectedTexture->x += dx;
					//selectedTexture->y += dy;
				}
			}
			this->update();
		}
	}

	localPos = curPos;
}

void WorkArea::mouseReleaseEvent(QMouseEvent *event)
{
	//selectedTexture=0;
	this->update();
}

void WorkArea::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Delete:
			if (selectedTexture)
			{
				qDebug("444");
				textureModel->delTexture(selectedTexture);
				selectedTexture=0;
			}
			break;
		default: QWidget::keyPressEvent(event);
	}
}

