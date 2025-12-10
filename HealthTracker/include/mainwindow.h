#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <optional>
#include "DatabaseManager.h"
/**
 * @file mainwindow.h
 * @brief Declaración de la clase MainWindow para la interfaz principal del sistema de salud.
 *
 * Esta clase administra la navegación entre páginas, el manejo de eventos de la UI,
 * la interacción con la base de datos y el procesamiento de datos de usuarios
 * y registros de salud. Incluye funciones para iniciar sesión, registrar usuarios,
 * guardar mediciones, exportar datos y generar análisis estadísticos.
 */
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_2_clicked();  // Crear Usuario (pantalla)
    void on_pushButton_3_clicked();  // Iniciar Sesión (pantalla)
    void on_pushButton_clicked();    // Salir inicial

    void on_pushButton_10_clicked(); // Crear usuario
    void on_pushButton_11_clicked(); // Login

    void on_pushButton_4_clicked();  // Agregar registro (ir a page_5)
    void on_pushButton_5_clicked();  // Listar registros (page_6)
    void on_pushButton_6_clicked();  // Analizar (page_7)
    void on_pushButton_7_clicked();  // CSV (page_8)
    void on_pushButton_8_clicked();  // Eliminar (page_9)
    void on_pushButton_9_clicked();  // Salir menú

    void on_pushButton_12_clicked(); // Registrar datos
    void on_pushButton_13_clicked(); // Analizar
    void on_pushButton_14_clicked(); // Guardar archivo (CSV)
    void on_pushButton_15_clicked(); // Eliminar registro


    void on_pushButton_16_clicked();
    void on_pushButton_17_clicked();
    void on_pushButton_18_clicked();
    void on_pushButton_19_clicked();
    void on_pushButton_20_clicked();
    void on_pushButton_21_clicked();

private:
    Ui::MainWindow *ui;
    DatabaseManager db;
    std::optional<User> current_user;

    void mostrarError(const QString &msg);
    void cargarRegistros();
};

#endif

