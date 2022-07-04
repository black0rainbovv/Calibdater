#include "inaccdialog.h"
#include "ui_inaccdialog.h"
#include "QPushButton"
#include "QDebug"

InaccDialog::InaccDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InaccDialog)
{
    ui->setupUi(this);
}

InaccDialog::~InaccDialog()
{
    delete ui;
}

QList<QPushButton*> blist;

void InaccDialog::SetTable(QVector<QStringList> params)
{
    blist.clear();
    QList<QColor> scolors;
    scolors << Qt::red << Qt::green << Qt::blue << Qt::cyan << Qt::magenta << Qt::yellow;
    this->setWindowTitle("Систематические отклонения");
    int rows = params[0].size()-1+2;
    if(rows > 12)
        this->setFixedSize(600, 12*25+60);
    else
        this->setFixedSize(600, rows*25+60);
    ui->tableWidget->setColumnCount(14);//params.size());
    ui->tableWidget->setRowCount(rows);//params[0].split(";").size()-1+2);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);

    QHeaderView *hheaderView = new QHeaderView(Qt::Horizontal, ui->tableWidget);
    QHeaderView *vheaderView = new QHeaderView(Qt::Vertical, ui->tableWidget);
    ui->tableWidget->setVerticalHeader(vheaderView);
    ui->tableWidget->setHorizontalHeader(hheaderView);
    ui->tableWidget->horizontalHeader()->hide();
    ui->tableWidget->verticalHeader()->hide();
    vheaderView->resizeSection(0, 25);
    vheaderView->resizeSection(1, 25);
    for(int i=2; i<rows; i++)
    {
        vheaderView->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    for(int i=0; i<13; i+=2)
    {
        ui->tableWidget->setSpan(0, i, 1, 2);
    }
    ui->tableWidget->setSpan(0, 12, 2, 1);
    ui->tableWidget->setSpan(0, 13, 2, 1);

    for(int i=0; i<12; i++)
    {
        hheaderView->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    hheaderView->resizeSection(12, 70);
    hheaderView->resizeSection(13, 25);

    for(int i=0; i<6; i++)
    {
        QTableWidgetItem *q = new QTableWidgetItem();
        q->setTextAlignment(Qt::AlignCenter);
        q->setText(QString::number(i+1));
        q->setBackground(QBrush(QColor(scolors[i])));
        q->setFont(QFont("MS Shell", 10));
        ui->tableWidget->setItem(0, 2*i, q);
    }
    QTableWidgetItem *d = new QTableWidgetItem();
    d->setText("Дата");
    d->setTextAlignment(Qt::AlignCenter);
    d->setBackground(Qt::gray);
    d->setFont(QFont("MS Shell", 0));
    ui->tableWidget->setItem(0, 12, d);
    for(int i=0; i<6; i++)
    {
        QTableWidgetItem *q20 = new QTableWidgetItem();
        q20->setTextAlignment(Qt::AlignCenter);
        q20->setText("20°C");
        q20->setBackground(Qt::gray);
        ui->tableWidget->setItem(1, 2*i, q20);
        QTableWidgetItem *q90 = new QTableWidgetItem();
        q90->setTextAlignment(Qt::AlignCenter);
        q90->setText("90°C");
        q90->setBackground(Qt::gray);
        ui->tableWidget->setItem(1, 2*i+1, q90);
    }


    for(int i=2; i<rows; i++)
    {
        QTableWidgetItem *d = new QTableWidgetItem();
        d->setTextAlignment(Qt::AlignCenter);
        d->setText(params[12][i-2]);
        ui->tableWidget->setItem(i, 12, d);
    }
    for(int i=2; i<rows; i++)
    {
        for(int j=0; j<6; j++)
        {
            QTableWidgetItem *q20 = new QTableWidgetItem();
            q20->setTextAlignment(Qt::AlignCenter);
            q20->setText(params[2*j][i-2]);
            ui->tableWidget->setItem(i, 2*j, q20);
            QTableWidgetItem *q90 = new QTableWidgetItem();
            q90->setTextAlignment(Qt::AlignCenter);
            q90->setText(params[2*j+1][i-2]);
            ui->tableWidget->setItem(i, 2*j+1, q90);
        }
    }
    for(int i=2; i<rows; i++)
    {
        QPushButton *p = new QPushButton(ui->tableWidget);
        p->setText("<");
        ui->tableWidget->setCellWidget(i, 13, p);
        blist << p;
        QObject::connect(p, SIGNAL(clicked()), this, SLOT(DeleteRow()));
    }
}

void InaccDialog::DeleteRow()
{
    QPushButton *b = qobject_cast<QPushButton*>(sender ());
    int row = blist.indexOf(b)+2;
    if(ui->tableWidget->rowCount() > 3)
    {
        qDebug() << row;
        ui->tableWidget->removeRow(row);
        this->setFixedHeight(this->height()-25);
    }
    blist.removeAt(row-2);
}

void InaccDialog::on_pushButton_clicked() //Отмена
{
    this->close();
}

void InaccDialog::on_pushButton_2_clicked() //Принять
{
    QStringList t;
    for(int j=0; j<13; j++)
    {
        for(int i=2; i<ui->tableWidget->rowCount(); i++)
        {
            QTableWidgetItem *q = new QTableWidgetItem;
            q = ui->tableWidget->item(i,j);
            t << q->text();
        }
        newparams << t;
        t.clear();
    }
    emit sendNewParams(newparams);
    this->close();
}
