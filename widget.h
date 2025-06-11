#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QList>
#include <QDate>
#include <QTableWidgetItem>
#include <QCloseEvent>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QMetaType>
#include <QMenu>
#include <QInputDialog>
#include <QFileDialog>
#include <QDateTime>
#include <QPalette>
#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QSizePolicy>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

struct AccountRecord {
    bool isIncome;
    double amount;
    QDate date;
    QString note;
};

Q_DECLARE_METATYPE(AccountRecord)

QDataStream &operator<<(QDataStream &out, const AccountRecord &v);
QDataStream &operator>>(QDataStream &in, AccountRecord &v);

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_incomeButton_clicked();
    void on_expenseButton_clicked();
    void on_filterButton_clicked();
    void on_saveButton_clicked();
    void on_loadButton_clicked();
    void on_exportButton_clicked();

    void on_categoryStatsButton_clicked();
    void on_compressButton_clicked();
    void on_restoreButton_clicked();

    void showRecordContextMenu(const QPoint &pos);
    void deleteRecord(int row);
    void editRecord(int row);

private:
    Ui::Widget *ui;

    QList<AccountRecord> allRecords;
    QList<AccountRecord> filteredRecords;

    void refreshTable();
    void updateSummary();
    void clearInputs();
    bool validateInput();

    void loadAutoBackup();
    void createAutoBackup();
    void logOperation(const QString &action);
    void saveToFile(const QString &fileName);
    void showCategoryStats();
    void loadFromFile(const QString &fileName);

    bool checkDataIntegrity();


    void compressData();
    void restoreBackup();

};

#endif // WIDGET_H
