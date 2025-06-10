#include "widget.h"
#include "ui_widget.h"


QDataStream &operator<<(QDataStream &out, const AccountRecord &v) {
    return out << v.isIncome
               << v.amount
               << v.date
               << v.note;
}

QDataStream &operator>>(QDataStream &in, AccountRecord &v) {
    return in >> v.isIncome
           >> v.amount
           >> v.date
           >> v.note;
}


void Widget::logOperation(const QString &action)
{

    QFile logFile("operations.log");

    if (logFile.open(QIODevice::Append | QIODevice::Text)) {

        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
            << " - " << action << "\n";


        logFile.close();
    }
}


void Widget::loadAutoBackup()
{

    QFile file("autobackup.dat");

    if (file.exists() && file.open(QIODevice::ReadOnly)) {

        QDataStream in(&file);
        in >> allRecords;
        file.close();

        filteredRecords = allRecords;
        refreshTable();

        logOperation("加载自动备份数据");
    }
}


void Widget::createAutoBackup()
{

    if (allRecords.isEmpty()) {
        return;
    }

    QFile file("autobackup.dat");

    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << allRecords;
        file.close();
    }
}


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{

    ui->setupUi(this);


    qRegisterMetaType<AccountRecord>("AccountRecord");


    ui->dateEdit->setDate(QDate::currentDate());

    ui->startDateEdit->setDate(QDate::currentDate().addMonths(-1));
    ui->endDateEdit->setDate(QDate::currentDate());

    QStringList headers;
    headers << "序号"
            << "类型"
            << "金额"
            << "日期"
            << "备注";

    ui->recordTable->setColumnCount(5);
    ui->recordTable->setHorizontalHeaderLabels(headers);

    ui->recordTable->horizontalHeader()->setSectionResizeMode(
        4,
        QHeaderView::Stretch
        );

    ui->recordTable->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(
        ui->recordTable,
        &QTableWidget::customContextMenuRequested,
        this,
        &Widget::showRecordContextMenu
        );

    connect(
        ui->recordTable,
        &QTableWidget::cellDoubleClicked,
        this,
        &Widget::editRecord
        );

    this->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
        );


    connect(
        ui->clearFilterButton,
        &QPushButton::clicked,
        [this]() {
            filteredRecords = allRecords;
            refreshTable();

            logOperation("清除筛选条件，显示全部记录");
        }
        );

    loadAutoBackup();
}



Widget::~Widget()
{
    createAutoBackup();

    delete ui;
}


void Widget::on_incomeButton_clicked()
{
    if (!validateInput()) {
        return;
    }

    AccountRecord record;
    record.isIncome = true;
    record.amount = ui->amountEdit->text().toDouble();
    record.date = ui->dateEdit->date();
    record.note = ui->noteEdit->text().trimmed();

    allRecords.append(record);
    filteredRecords = allRecords;

    refreshTable();

    clearInputs();

    logOperation(
        QString("添加收入记录: 金额 %1, 备注: %2")
            .arg(record.amount)
            .arg(record.note)
        );
}

void Widget::on_expenseButton_clicked()
{
    if (!validateInput()) {
        return;
    }

    AccountRecord record;
    record.isIncome = false;
    record.amount = ui->amountEdit->text().toDouble();
    record.date = ui->dateEdit->date();
    record.note = ui->noteEdit->text().trimmed();

    allRecords.append(record);
    filteredRecords = allRecords;

    refreshTable();

    clearInputs();

    logOperation(
        QString("添加支出记录: 金额 %1, 备注: %2")
            .arg(record.amount)
            .arg(record.note)
        );
}


void Widget::on_filterButton_clicked()
{
    QDate start = ui->startDateEdit->date();
    QDate end = ui->endDateEdit->date();

    if (start > end) {
        QMessageBox::warning(
            this,
            "日期错误",
            "开始日期不能晚于结束日期"
            );
        return;
    }

    filteredRecords.clear();

    for (const AccountRecord &record : allRecords) {
        if (record.date >= start && record.date <= end) {
            filteredRecords.append(record);
        }
    }

    refreshTable();

    logOperation(
        QString("筛选记录: %1 至 %2, 共 %3 条记录")
            .arg(start.toString("yyyy-MM-dd"))
            .arg(end.toString("yyyy-MM-dd"))
            .arg(filteredRecords.size())
        );
}


void Widget::refreshTable()
{
    ui->recordTable->setRowCount(0);

    for (int i = 0; i < filteredRecords.size(); ++i) {
        const AccountRecord &record = filteredRecords[i];


        int row = ui->recordTable->rowCount();
        ui->recordTable->insertRow(row);

        ui->recordTable->setItem(
            row,
            0,
            new QTableWidgetItem(QString::number(row + 1))
            );

        ui->recordTable->setItem(
            row,
            1,
            new QTableWidgetItem(record.isIncome ? "收入" : "支出")
            );

        ui->recordTable->setItem(
            row,
            2,
            new QTableWidgetItem(QString::number(record.amount, 'f', 2))
            );

        ui->recordTable->setItem(
            row,
            3,
            new QTableWidgetItem(record.date.toString("yyyy-MM-dd"))
            );

        ui->recordTable->setItem(
            row,
            4,
            new QTableWidgetItem(record.note)
            );

        QColor textColor = record.isIncome
                               ? QColor(0, 128, 0)
                               : QColor(200, 0, 0);

        for (int col = 0; col < 5; ++col) {
            if (ui->recordTable->item(row, col)) {
                ui->recordTable->item(row, col)->setForeground(
                    QBrush(textColor)
                    );
            }
        }
    }

    updateSummary();
}

void Widget::updateSummary()
{
    double totalIncome = 0;
    double totalExpense = 0;

    for (const AccountRecord &record : filteredRecords) {
        if (record.isIncome) {
            totalIncome += record.amount;
        } else {
            totalExpense += record.amount;
        }
    }

    ui->totalIncomeLabel->setText(
        QString::number(totalIncome, 'f', 2)
        );

    ui->totalExpenseLabel->setText(
        QString::number(totalExpense, 'f', 2)
        );

    ui->balanceLabel->setText(
        QString::number(totalIncome - totalExpense, 'f', 2)
        );

    QPalette palette = ui->balanceLabel->palette();
    palette.setColor(
        QPalette::WindowText,
        (totalIncome - totalExpense) >= 0 ? Qt::darkGreen : Qt::red
        );

    ui->balanceLabel->setPalette(palette);
}


void Widget::clearInputs()
{
    ui->amountEdit->clear();
    ui->noteEdit->clear();

    ui->dateEdit->setDate(QDate::currentDate());

    ui->amountEdit->setFocus();
}

bool Widget::validateInput()
{
    if (ui->amountEdit->text().isEmpty()) {
        QMessageBox::warning(
            this,
            "输入错误",
            "金额不能为空"
            );

        ui->amountEdit->setFocus();
        return false;
    }

    bool ok;
    double amount = ui->amountEdit->text().toDouble(&ok);

    if (!ok || amount <= 0) {
        QMessageBox::warning(
            this,
            "输入错误",
            "请输入有效的金额（大于0的数字）"
            );

        ui->amountEdit->selectAll();
        ui->amountEdit->setFocus();
        return false;
    }

    if (ui->noteEdit->text().length() > 100) {
        QMessageBox::warning(
            this,
            "输入错误",
            "备注长度不能超过100个字符"
            );

        ui->noteEdit->selectAll();
        ui->noteEdit->setFocus();
        return false;
    }

    if (ui->dateEdit->date() > QDate::currentDate()) {
        QMessageBox::warning(
            this,
            "日期错误",
            "日期不能晚于今天"
            );

        ui->dateEdit->setFocus();
        return false;
    }

    return true;
}


void Widget::on_saveButton_clicked()
{
    saveToFile("records.dat");
}


void Widget::on_loadButton_clicked()
{
    loadFromFile("records.dat");
}


void Widget::on_exportButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出CSV文件",
        "",
        "CSV文件 (*.csv)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(
            this,
            "错误",
            "无法创建CSV文件"
            );
        return;
    }

    QTextStream out(&file);
    out << "类型,金额,日期,备注\n";

    for (const AccountRecord &record : filteredRecords) {
        out << (record.isIncome ? "收入" : "支出") << ","
            << QString::number(record.amount, 'f', 2) << ","
            << record.date.toString("yyyy-MM-dd") << ","
            << "\"" << record.note << "\"" << "\n";
    }

    file.close();

    QMessageBox::information(
        this,
        "导出成功",
        "数据已导出为CSV格式"
        );

    logOperation(
        QString("导出CSV文件: %1").arg(fileName)
        );
}


void Widget::saveToFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(
            this,
            "错误",
            QString("无法创建文件: %1").arg(fileName)
            );
        return;
    }

    QDataStream out(&file);
    out << allRecords;

    file.close();

    QMessageBox::information(
        this,
        "保存成功",
        QString("数据已保存到 %1").arg(fileName)
        );

    logOperation(
        QString("保存数据到: %1").arg(fileName)
        );
}


void Widget::loadFromFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(
            this,
            "错误",
            QString("找不到数据文件: %1").arg(fileName)
            );
        return;
    }

    QDataStream in(&file);
    in >> allRecords;

    file.close();

    filteredRecords = allRecords;
    refreshTable();

    QMessageBox::information(
        this,
        "加载成功",
        QString("已加载 %1 条记录").arg(allRecords.size())
        );

    logOperation(
        QString("从文件加载数据: %1").arg(fileName)
        );
}


void Widget::deleteRecord(int row)
{
    if (row < 0 || row >= filteredRecords.size()) {
        return;
    }

    AccountRecord record = filteredRecords[row];

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除该记录吗?\n金额: %1\n日期: %2\n备注: %3")
            .arg(record.amount)
            .arg(record.date.toString("yyyy-MM-dd"))
            .arg(record.note),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        for (int i = 0; i < allRecords.size(); ++i) {
            if (allRecords[i].date == record.date &&
                allRecords[i].amount == record.amount &&
                allRecords[i].note == record.note) {
                allRecords.removeAt(i);
                break;
            }
        }

        filteredRecords.removeAt(row);

        refreshTable();

        logOperation(
            QString("删除记录: 金额 %1, 日期 %2, 备注: %3")
                .arg(record.amount)
                .arg(record.date.toString("yyyy-MM-dd"))
                .arg(record.note)
            );
    }
}


void Widget::editRecord(int row)
{
    if (row < 0 || row >= filteredRecords.size()) {
        return;
    }

    AccountRecord record = filteredRecords[row];

    bool ok;
    QString newNote = QInputDialog::getText(
        this,
        "编辑备注",
        "请输入新的备注:",
        QLineEdit::Normal,
        record.note,
        &ok
        );

    if (ok && !newNote.isEmpty()) {
        for (int i = 0; i < allRecords.size(); ++i) {
            if (allRecords[i].date == record.date &&
                allRecords[i].amount == record.amount &&
                allRecords[i].note == record.note) {
                allRecords[i].note = newNote;
                break;
            }
        }

        filteredRecords[row].note = newNote;

        refreshTable();

        logOperation(
            QString("编辑记录: 原备注 [%1] 改为 [%2]")
                .arg(record.note, newNote)
            );
    }
}


void Widget::showRecordContextMenu(const QPoint &pos)
{
    QTableWidget *table = ui->recordTable;
    QModelIndex index = table->indexAt(pos);

    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);
    QAction *editAction = menu.addAction("编辑备注");
    QAction *deleteAction = menu.addAction("删除记录");

    QAction *selectedAction = menu.exec(
        table->viewport()->mapToGlobal(pos)
        );

    if (selectedAction == deleteAction) {
        deleteRecord(index.row());
    }
    else if (selectedAction == editAction) {
        editRecord(index.row());
    }
}






void Widget::showCategoryStats()
{
    QMap<QString, double> incomeCategories;
    QMap<QString, double> expenseCategories;

    for (const AccountRecord &record : allRecords) {
        if (record.isIncome) {
            incomeCategories[record.note] += record.amount;
        } else {
            expenseCategories[record.note] += record.amount;
        }
    }

    QString stats = "===== 收入分类统计 =====\n";

    for (auto it = incomeCategories.begin(); it != incomeCategories.end(); ++it) {
        stats += QString("%1: %2\n")
        .arg(it.key())
            .arg(it.value(), 0, 'f', 2);
    }

    stats += "\n===== 支出分类统计 =====\n";

    for (auto it = expenseCategories.begin(); it != expenseCategories.end(); ++it) {
        stats += QString("%1: %2\n")
        .arg(it.key())
            .arg(it.value(), 0, 'f', 2);
    }

    QMessageBox::information(
        this,
        "分类统计",
        stats
        );

    logOperation("生成分类统计报表");
}


void Widget::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "确认退出",
        "确定要退出程序吗?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        createAutoBackup();

        event->accept();
    } else {
        event->ignore();
    }
}


bool Widget::checkDataIntegrity()
{
    if (allRecords.isEmpty()) {
        qDebug() << "数据完整性检查: 无记录";
        return true;
    }

    int errors = 0;

    for (const AccountRecord &record : allRecords) {
        if (record.amount <= 0) {
            qWarning() << "无效金额记录:" << record.amount;
            errors++;
        }

        if (record.date.isNull() || !record.date.isValid()) {
            qWarning() << "无效日期记录:" << record.date;
            errors++;
        }
    }

    if (errors > 0) {
        qCritical() << "发现" << errors << "个数据完整性问题";
        return false;
    }

    qDebug() << "数据完整性检查通过";
    return true;
}


void Widget::compressData()
{
    int originalCount = allRecords.size();

    QList<AccountRecord> compressed;

    for (const AccountRecord &record : allRecords) {
        bool found = false;

        for (AccountRecord &comp : compressed) {
            if (comp.date == record.date &&
                comp.note == record.note &&
                comp.isIncome == record.isIncome) {
                comp.amount += record.amount;
                found = true;
                break;
            }
        }

        if (!found) {
            compressed.append(record);
        }
    }

    int compressedCount = originalCount - compressed.size();

    if (compressedCount > 0) {
        allRecords = compressed;
        filteredRecords = allRecords;

        refreshTable();

        QMessageBox::information(
            this,
            "数据压缩",
            QString("压缩了 %1 条重复记录").arg(compressedCount)
            );
    } else {
        QMessageBox::information(
            this,
            "数据压缩",
            "未找到可压缩的重复记录"
            );
    }
}


QString Widget::formatCurrency(double amount)
{
    return QString("¥%1").arg(
        QString::number(amount, 'f', 2)
        );
}


void Widget::restoreBackup()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "恢复备份",
        "确定要恢复最近一次备份吗?\n当前数据将会丢失",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        loadFromFile("autobackup.dat");
    }
}


void Widget::on_categoryStatsButton_clicked()
{
    showCategoryStats();
}


void Widget::on_compressButton_clicked()
{
    compressData();
}


void Widget::on_restoreButton_clicked()
{
    restoreBackup();
}
