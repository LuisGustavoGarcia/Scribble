#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "getdirectorywindow.h"
#include "addcoursewindow.h"
#include "chapterinfowindow.h"
#include "addnotes.h"
#include <QFileInfo>
#include <QFile>
#include <QDirIterator>
#include <QDebug>

//having issues if i declate it in header
ChapterInfoWindow *infoWindow = NULL;
AddNotes *noteWindow = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    currentItem = NULL;
    groupBoxLayout = NULL;
    projectPath = "";

    initializeCourseList();
    loadCourseList();

}
bool MainWindow::initializeCourseList(){
    if(QFileInfo::exists(pathname)){
        QFile file(pathname);
         if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            //file didnt open, ask user to set new directory
             GetDirectoryWindow window;
             window.setModal(true);
             window.exec();
         }

        projectPath = file.readAll();

        file.close();

        //go to project path and check for course dir
        QDir projectDir(projectPath);
        QDirIterator iter(projectDir,QDirIterator::NoIteratorFlags);
        while(iter.hasNext()){
            iter.next();
            QString filename = iter.fileName();
            if(filename != "." && filename != ".."){
                Course *course = new Course();
                course->setCourseName(filename);
                courses.addCourse(course);
            }

        }

        return true;
    }else{
        //ask user to set the project directory if it doesnt exist
        GetDirectoryWindow window;
        window.setModal(true);
        window.exec();

        return initializeCourseList();
    }
}
//Add Course
QTreeWidgetItem* MainWindow::addRoot(QString name){
    //make treewidget item and set its parent to the treewidget
    QTreeWidgetItem *course = new QTreeWidgetItem(ui->courseList);
    //first parameter is the column number, the second is its name
    course->setText(0,name);

    return course;

}
//Add Textbook
QTreeWidgetItem* MainWindow::addChild(QTreeWidgetItem *parent, QString name){
    QTreeWidgetItem *itm = new QTreeWidgetItem();
    itm->setText(0,name);

    parent->addChild(itm);

    return itm;
}

//Add the courses to the treewidget as roots
void MainWindow::loadCourseList(){

        QTreeWidgetItem* root = NULL;
        //go to project path and check for course dir
        QDir projectDir(projectPath);
        QDirIterator iter(projectDir,QDirIterator::Subdirectories);
        while(iter.hasNext()){
            iter.next();
            //qDebug() << "File Path: " << iter.fileName();
            QString filename = iter.fileName();
            if(filename != "." && filename != ".." && courses.courseExist(filename)){
                //its a root
                root = addRoot(filename);
            }else if(filename != "." && filename != ".." ){
                //not a root
                root = addChild(root,filename);
            }

        }
}

MainWindow::~MainWindow()
{
    delete ui;
}



//adds course to the tree widget and creates a directory for it
void MainWindow::on_addCourseBtn_clicked()
{
    AddCourseWindow window;
    window.setModal(true);
    window.exec();

    //get course and textbook form dialog window
    QString course = window.getCourseName();
    QString textbook = window.getTextbookName();
    QString chapter = window.getChapterName();
    //make sure it isnt blank
    if(!course.isEmpty() && !textbook.isEmpty() && !chapter.isEmpty()){
        QTreeWidgetItem* courseRoot = addRoot(course);
        QTreeWidgetItem* textbookItem = addChild(courseRoot,textbook);
        addChild(textbookItem,chapter);
    }

    //----------------TODO------------------------------
    //----------------------Make the directories for these in the
    //project folder----------------------------------------------------
}

void MainWindow::on_courseList_itemClicked(QTreeWidgetItem *item, int column)
{

    if(item->parent() != NULL){
        int nestNum = countNest(item);
       if(nestNum == 2){
            currentItem = item;
           //its a chapter
           if(noteWindow != NULL)
               noteWindow->hide();
           if(infoWindow != NULL){
               infoWindow->show();
           }else{
               //make container for widgets
                groupBoxLayout = new QVBoxLayout();
                infoWindow = new ChapterInfoWindow();
                infoWindow->setMainWidget(this);
                groupBoxLayout->addWidget(infoWindow);

                ui->groupBox->setLayout(groupBoxLayout);
           }
       }else if(nestNum == 3){
           //its a note
           if(infoWindow != NULL)
                infoWindow->hide();
           if(noteWindow != NULL){
               noteWindow->show();
           }else{
                noteWindow = new AddNotes();
                groupBoxLayout->addWidget(noteWindow);
           }

       }
    }
}

int MainWindow::countNest(QTreeWidgetItem *parent){
    int count  = 0;
   while(parent->parent() != NULL){
       count++;
       parent = parent->parent();
   }

   return count;
}

QTreeWidgetItem* MainWindow::getCurrentItem(){
    return currentItem;
}
