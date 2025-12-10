#include "CSVExporter.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
using namespace std;

bool CSVExporter::export_file(const std::vector<HealthRecord>& records, const std::string& filename, const std::string& username, std::string& out_err, bool generate_graphs) {
    ofstream ofs(filename);
    if (!ofs) {
        out_err = "no se pudo abrir archivo para escribir";
        return false;
    }

    ofs << "usuario,fecha,hora,peso,sistolico,diastolico,glucosa\n";
    for (const auto& r : records) {
        size_t space_pos = r.datetime.find(' ');
        string date = (space_pos != string::npos) ? r.datetime.substr(0, space_pos) : r.datetime;
        string time = (space_pos != string::npos) ? r.datetime.substr(space_pos + 1) : "";
        ofs << username << ",\"" << date << "\",\"" << time << "\"," << r.weight << "," << r.systolic << "," << r.diastolic << "," << r.glucose << "\n";
    }
    ofs.close();

    if (generate_graphs) {
        
        string data_filename = filename.substr(0, filename.find_last_of('.')) + ".dat";
        ofstream data_ofs(data_filename);
        if (!data_ofs) {
            out_err = "no se pudo abrir archivo de datos para escribir";
            return false;
        }
        for (const auto& r : records) {
            data_ofs << r.datetime << " " << r.weight << " " << r.systolic << " " << r.diastolic << " " << r.glucose << "\n";
        }
        data_ofs.close();

        
        vector<string> metrics = {"Peso", "Sistolico", "Diastolico", "Glucosa"};
        vector<int> columns = {2, 3, 4, 5};
        for (size_t i = 0; i < metrics.size(); ++i) {
            string script_filename = filename.substr(0, filename.find_last_of('.')) + "_" + metrics[i] + ".gp";
            ofstream script_ofs(script_filename);
            if (!script_ofs) {
                out_err = "no se pudo abrir archivo de script para escribir";
                return false;
            }
            string png_filename = filename.substr(0, filename.find_last_of('.')) + "_" + metrics[i] + ".png";
            script_ofs << "set terminal pngcairo enhanced font 'Arial,12' size 800,600\n";
            script_ofs << "set output '" << png_filename << "'\n";
            script_ofs << "set xdata time\n";
            script_ofs << "set timefmt '%Y-%m-%d %H:%M:%S'\n";
            script_ofs << "set format x '%H:%M:%S'\n";
            script_ofs << "set xlabel 'Tiempo'\n";
            script_ofs << "set ylabel '" << metrics[i] << "'\n";
            script_ofs << "set title 'Evolución de " << metrics[i] << " a lo largo del tiempo'\n";
            script_ofs << "set grid\n";
            script_ofs << "set style line 1 lc rgb '#0072BD' lt 1 lw 2 pt 7 ps 1\n";
            script_ofs << "plot '" << data_filename << "' using 1:" << columns[i] << " title '" << metrics[i] << "' with linespoints ls 1\n";
            script_ofs.close();

            
            string command = "gnuplot " + script_filename;
            int result = system(command.c_str());
            if (result != 0) {
                out_err = "error al ejecutar gnuplot para " + metrics[i];
                return false;
            }
        }
    }

    return true;
}
