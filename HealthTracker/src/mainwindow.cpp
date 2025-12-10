#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    if(!db.init("health.db")){
        QMessageBox::critical(this, "Error", "No se pudo abrir la base de datos");
    }

    ui->stackedWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::mostrarError(const QString &msg) {
    QMessageBox::warning(this, "Error", msg);
}

void MainWindow::on_pushButton_2_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_3_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_pushButton_clicked() {
    close();
}

void MainWindow::on_pushButton_10_clicked() {
    QString user = ui->lineEdit->text();
    QString pass = ui->lineEdit_2->text();

    std::string err;
    if (db.create_user(user.toStdString(), pass.toStdString(), err)) {
        ui->textEdit_19->setText("Usuario creado correctamente");
        ui->stackedWidget->setCurrentIndex(0);
    } else {
        ui->textEdit_19->setText(QString::fromStdString(err));
    }
}


void MainWindow::on_pushButton_11_clicked() {
    QString user = ui->lineEdit_3->text();
    QString pass = ui->lineEdit_4->text();

    std::string err;
    auto maybe = db.verify_user(user.toStdString(), pass.toStdString(), err);

    if(maybe){
        current_user = *maybe;
        ui->stackedWidget->setCurrentIndex(3);
    } else {
        ui->textEdit_18->setText("Usuario o contraseña incorrecto");
    }
}

void MainWindow::on_pushButton_4_clicked() {
    ui->stackedWidget->setCurrentIndex(4);
}

void MainWindow::on_pushButton_5_clicked() {
    ui->stackedWidget->setCurrentIndex(5);
    cargarRegistros();
}

void MainWindow::on_pushButton_6_clicked() {
    ui->stackedWidget->setCurrentIndex(6);
}

void MainWindow::on_pushButton_7_clicked() {
    ui->stackedWidget->setCurrentIndex(7);
}

void MainWindow::on_pushButton_8_clicked() {
    ui->stackedWidget->setCurrentIndex(8);
}

void MainWindow::on_pushButton_9_clicked() {
    current_user.reset();
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_pushButton_12_clicked() {
    if (!current_user) {
        mostrarError("No hay usuario autenticado");
        return;
    }

    std::string err;

    HealthRecord r;
    r.user_id = current_user->id;
    r.datetime = HealthRecord::now_date();
    r.weight = ui->lineEdit_5->text().toDouble();
    r.systolic = ui->lineEdit_6->text().toInt();
    r.diastolic = ui->lineEdit_7->text().toInt();
    r.glucose = ui->lineEdit_8->text().toDouble();

    if(db.add_record(r, err)){
        ui->textEdit_17->setText("Registro guardado");
    } else {
        ui->textEdit_17->setText(QString::fromStdString(err));
    }
}

void MainWindow::on_pushButton_13_clicked() {
    if (!current_user) {
        mostrarError("No hay usuario autenticado");
        return;
    }

    std::string err;
    std::string text;

    if (db.generate_analysis(current_user->id, text, err)) {
        ui->textEdit_16->setPlainText(QString::fromStdString(text));
        ui->stackedWidget->setCurrentIndex(6);
    } else {
        mostrarError(QString::fromStdString(err));
    }
}


void MainWindow::on_pushButton_14_clicked() {
    if (!current_user) {
        mostrarError("No hay usuario autenticado");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, "Guardar CSV", "datos.csv", "Archivos CSV (*.csv)");
    if (filename.isEmpty()) return;

    std::string err;
    if (db.export_csv(current_user->id, filename.toStdString(), err)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
    } else {
        mostrarError(QString::fromStdString(err));
    }
}

void MainWindow::on_pushButton_15_clicked() {
    if (!current_user) {
        mostrarError("No hay usuario autenticado");
        return;
    }

    int id = ui->lineEdit_9->text().toInt();
    std::string err;
    if(db.delete_record(id, current_user->id, err)){
        ui->textEdit_45->setText("Registro eliminado");
        cargarRegistros();
    } else {
        ui->textEdit_45->setText(QString::fromStdString(err));
    }
}

// botones "volver a inicio" en varias páginas: todos llevan a la página 3 (menú principal)
void MainWindow::on_pushButton_16_clicked() { ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_pushButton_17_clicked() { ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_pushButton_18_clicked() { ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_pushButton_19_clicked() { ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_pushButton_20_clicked() { ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_pushButton_21_clicked() { ui->stackedWidget->setCurrentIndex(3); }

void MainWindow::cargarRegistros() {
    if (!current_user) {
        mostrarError("No hay usuario autenticado");
        return;
    }

    std::string err;
    auto registros = db.get_ruser(current_user->id, err);

    if (!err.empty()) {
        mostrarError(QString::fromStdString(err));
        return;
    }

    QTableWidget *table = ui->tableRegistros;
    if (!table) {
        mostrarError("No se encontró la tabla 'tableRegistros' en la interfaz");
        return;
    }

    table->clear();
    table->setRowCount(static_cast<int>(registros.size()));
    table->setColumnCount(6);

    QStringList headers;
    headers << "ID" << "Fecha" << "Peso" << "Sistólica" << "Diastólica" << "Glucosa";
    table->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < static_cast<int>(registros.size()); ++i) {
        const auto &r = registros[i];

        table->setItem(i, 0, new QTableWidgetItem(QString::number(r.id)));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(r.datetime)));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(r.weight)));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(r.systolic)));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(r.diastolic)));
        table->setItem(i, 5, new QTableWidgetItem(QString::number(r.glucose)));
    }

    table->resizeColumnsToContents();
}


