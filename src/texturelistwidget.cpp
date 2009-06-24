#include "texturelistwidget.h"

TextureListWidget::TextureListWidget(QWidget *parent):QListView(parent)
{
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
}

//////////////////////
void TextureListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("image/x-texture"))
	event->accept();
    else
	event->acceptProposedAction();
    //event->ignore();
}

void TextureListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("image/x-texture"))
    {
	event->setDropAction(Qt::MoveAction);
    	event->accept();
    }
    else
	 event->acceptProposedAction();
    //event->ignore();
}

void TextureListWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("image/x-texture"))
    {
	QByteArray pieceData = event->mimeData()->data("image/x-texture");
	QDataStream dataStream(&pieceData, QIODevice::ReadOnly);
	QPixmap pixmap;
	//QPoint location;
	//dataStream >> pixmap >> location;
	//dataStream >> pixmap;

	// addPiece(pixmap, location);

	event->setDropAction(Qt::MoveAction);
	event->accept();
    }
    else
    {
	if (event->mimeData()->hasUrls())
	{
	    QList<QUrl> urlList = event->mimeData()->urls();
	    QString text;
	    for (int i = 0; i < urlList.size() && i < 32; ++i)
	    {
			QString url = urlList.at(i).toLocalFile();
			static_cast<TextureModel *>( this->model())->addTexture(url);
	    }
	    event->setDropAction(Qt::MoveAction);
	    event->accept();
	}
	else
	    event->ignore();
    }
}


void TextureListWidget::startDrag(Qt::DropActions supportedActions)
{
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    //QPixmap pixmap = qVariantValue<QPixmap>(item->data(Qt::UserRole));
    //QPixmap pixmap = qVariantValue<QPixmap>(this->currentIndex().data(Qt::UserRole));
    QPixmap pixmap = qVariantValue<QPixmap>(this->model()->data(this->currentIndex(),Qt::DecorationRole));

    //QPoint location = item->data(Qt::UserRole+1).toPoint();
    //dataStream << pixmap << location;
	int num = qVariantValue<int>(this->model()->data(this->currentIndex(),Qt::UserRole+1));
	dataStream << pixmap << num;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("image/x-texture", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));
    drag->setPixmap(pixmap);

    drag->exec(Qt::MoveAction);
    //if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
      //  delete takeItem(row(item));
}

void TextureListWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Delete:
			if (this->currentIndex().isValid())
				static_cast<TextureModel *>( this->model())->delTexture(this->currentIndex().row());
			break;
		default: QWidget::keyPressEvent(event);
	}
}

