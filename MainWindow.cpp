/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "PixelDelegate.h"
#include "ImageModel.h"
#include "KColorCells.h"
#include "KColorPatch.h"
#include "ViewMonitor.h"
#include "CommandsModel.h"
#include "CommandDelegate.h"

#include <QHBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QImage>
#include <QFileDialog>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMessageBox>
#include <QWheelEvent>
#include <QDebug>
#include <QListView>

static const int INITIAL_CODEL_SIZE = 12;

MainWindow::MainWindow( QWidget *parent ) :
        QMainWindow( parent ),
        ui( new Ui::MainWindow )
{
    ui->setupUi( this );

    mModel = new ImageModel;
    ui->mView->setModel( mModel );

    ui->mView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->mView->horizontalHeader()->hide();
    ui->mView->verticalHeader()->hide();
    ui->mView->horizontalHeader()->setMinimumSectionSize( 1 );
    ui->mView->verticalHeader()->setMinimumSectionSize( 1 );
    ui->mView->verticalHeader()->setResizeMode( QHeaderView::Fixed );
    ui->mView->horizontalHeader()->setResizeMode( QHeaderView::Fixed );
    ui->mView->horizontalHeader()->setDefaultSectionSize( 12 );
    ui->mView->verticalHeader()->setDefaultSectionSize( 12 );
    ui->mView->viewport()->installEventFilter( this );

    mMonitor = new ViewMonitor( this );
    mMonitor->setPixelSize( INITIAL_CODEL_SIZE );

    mDelegate = new PixelDelegate( mMonitor, this );
    ui->mView->setItemDelegate( mDelegate );
    setupDock();

    // setup save message
    mExtensions[ tr( "PNG (*.png)" )] = ".png";
    mExtensions[ tr( "GIF (*.gif)" )] = ".gif";
    mExtensions[ tr( "Portable Pixmap (*.ppm)" )] = ".ppm";

    QHashIterator<QString, QString> it( mExtensions );
    while ( it.hasNext() ) {
        it.next();
        mSaveMessage += it.key();
        if ( it.hasNext() )
            mSaveMessage += ";;";
    }

    connect( ui->mSpinBox, SIGNAL( valueChanged( int ) ), mMonitor, SLOT( setPixelSize( int ) ) );
    connect( mMonitor, SIGNAL( pixelSizeChanged( int ) ), SLOT( slotUpdateView( int ) ) );

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupDock()
{
    QWidget *colorsWidget = new QWidget( ui->mDockContents );
    QHBoxLayout *layout = new QHBoxLayout( colorsWidget  );
    mPalette = new KColorCells( this, 6, 3 );
    mPalette->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    mPalette->setFixedSize( 25*3, 25*6 );
    mMonitor->populateCells( mPalette );

//     mPatch = new KColorPatch( this );
//     mPatch->setFixedSize( 48, 48 );
//     mPatch->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

//     layout->addWidget( mPatch );
    layout->addWidget( mPalette );
    layout->setSpacing( 0 );
    colorsWidget->setLayout( layout );

//     QWidget *commandsWidget = new QWidget( ui->mDockContents );
    CommandsModel *commandsModel = new CommandsModel( mMonitor, this );
    QTableView *commandsView = new QTableView( ui->mDockContents );
    commandsView->horizontalHeader()->hide();
    commandsView->verticalHeader()->hide();
//     commandsView->horizontalHeader()->setMinimumSectionSize( 1 );
//     commandsView->verticalHeader()->setMinimumSectionSize( 1 );
//     commandsView->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );
    commandsView->horizontalHeader()->setResizeMode( QHeaderView::ResizeToContents );
//     commandsView->horizontalHeader()->setDefaultSectionSize( 50 );
//     commandsView->verticalHeader()->setDefaultSectionSize( 50 );
    commandsView->setModel( commandsModel );

    mCommandDelegate = new CommandDelegate( mMonitor, this );
    commandsView->setItemDelegate( mCommandDelegate );

    QBoxLayout *boxLayout = static_cast<QBoxLayout*>( ui->mDockContents->layout() );
    boxLayout->insertWidget(0, colorsWidget);
    boxLayout->insertWidget(1, commandsView);

    connect( mPalette, SIGNAL( colorSelected( int, QColor ) ), mMonitor, SLOT( setCurrentColor( int, QColor ) ) );
    connect( mMonitor, SIGNAL( currentColorChanged( int , QColor ) ), commandsView, SLOT( reset() ) );
}

void MainWindow::setupInstructions()
{

}

bool MainWindow::eventFilter( QObject* obj, QEvent* event )
{
    if ( obj == ui->mView->viewport() && event->type() == QEvent::Wheel ) {
        QWheelEvent * wevent = static_cast<QWheelEvent*>( event );
        // Ctrl + Wheel : change codel size
        if ( wevent->modifiers() == Qt::ControlModifier ) {
            const int numDegrees = wevent->delta() / 8;
            const int numSteps = numDegrees / 15;
            if ( wevent->orientation() == Qt::Vertical ) {
                ui->mSpinBox->setValue( ui->mSpinBox->value() + numSteps );
            }
            return true;
        }
    }
    return QObject::eventFilter( obj, event );
}

void MainWindow::on_actionToggleGrid_triggered()
{
    ui->mView->setShowGrid( !ui->mView->showGrid() );
}

void MainWindow::slotUpdateView( int pixelSize )
{
    ui->mView->resizeColumnsToContents();
    ui->mView->resizeRowsToContents();
    if ( ui->mView->horizontalHeader()->isVisible() )
        mModel->slotPixelSizeChange( pixelSize );
}

void MainWindow::on_actionOpenSource_triggered()
{

    QString file_name = QFileDialog::getOpenFileName( this, tr( "Open Image File" ),
                        QDesktopServices::storageLocation( QDesktopServices::HomeLocation ),
                        tr( "Images (*.png *.bmp *.ppm *.gif)" ) );
    QImage image( file_name );
    if ( !image.isNull() )
        mModel->setImage( image );

}

void MainWindow::on_actionSaveSource_triggered()
{
    QImage image = mModel->image();

    if ( image.isNull() )
        return;

    QString selected_filter;
    QString file_name = QFileDialog::getSaveFileName( this, tr( "Choose a file to save to" ),
                        QDesktopServices::storageLocation( QDesktopServices::HomeLocation ),
                        mSaveMessage,
                        &selected_filter );
    if ( file_name.isEmpty() )
        return;
    QFileInfo file_info( file_name );
    if ( file_info.suffix().isEmpty() )
        file_name.append( mExtensions[selected_filter] );

    if ( !image.save( file_name, 0 ) )
        QMessageBox::critical( this, tr( "Error saving image" ), tr( "An error occured when trying to save the image." ) );
}

void MainWindow::on_actionNew_triggered()
{
    QImage image( 50, 50, QImage::Format_RGB32 );
    image.fill( QColor( Qt::white ).rgb() );
    mModel->setImage( image, 1 );
}


void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionToggleHeaders_triggered()
{
    if ( ui->mView->horizontalHeader()->isVisible() ) { //hide
        ui->mView->horizontalHeader()->hide();
        ui->mView->verticalHeader()->hide();
        ui->mView->horizontalHeader()->setMinimumSectionSize( 1 );
        ui->mView->verticalHeader()->setMinimumSectionSize( 1 );
        ui->mView->verticalHeader()->setResizeMode( QHeaderView::Fixed );
        ui->mView->horizontalHeader()->setResizeMode( QHeaderView::Fixed );
        ui->mView->horizontalHeader()->setDefaultSectionSize( mMonitor->pixelSize() );
        ui->mView->verticalHeader()->setDefaultSectionSize( mMonitor->pixelSize() );
        ui->mSpinBox->setMinimum( 4 );
        mModel->slotPixelSizeChange( 1 );
    }  else { //show
        ui->mView->horizontalHeader()->show();
        ui->mView->verticalHeader()->show();
        ui->mView->verticalHeader()->setResizeMode( QHeaderView::Fixed );
        ui->mView->horizontalHeader()->setResizeMode( QHeaderView::Fixed );

        int rows = mModel->rowCount();
        if ( rows == 0 )
            return;
        int num_digits;
        while ( rows > 0 ) {
            ++num_digits;
            rows /= 10;
        }
        int largest_index = num_digits * 10; // not the largest index, but one of the largest (most digits)
        ui->mSpinBox->setValue( ui->mView->horizontalHeader()->sectionSize( largest_index ) );
        ui->mSpinBox->setMinimum( ui->mView->horizontalHeader()->sectionSize( largest_index ) );
        ui->mView->horizontalHeader()->setMinimumSectionSize( largest_index );
        ui->mView->verticalHeader()->setMinimumSectionSize( largest_index );
    }
}

#include "MainWindow.moc"
