#pragma once

#include <string>


// Struct que guarda os nomes dos arquivos usado no programa:
struct FilesNames{
    std::string bias_json;
    std::string params_json;
    std::string output_file;
    std::string path_data; 

    FilesNames(): bias_json("../settings.json"), params_json("../params.json"), output_file("../data_calibration"), path_data("/home/moa/projects/calib_evecam/calib_eventcam/") {}
};



struct Bias {
// Definição dos parâmetros que guardam os valores máximos e mínimos dos biases da
// câmera de eventos, eles são usados para validar os dados lidos do arquivo JSON antes de serem gravados na câmera.
// Os biases max e min. são definidos em https://docs.prophesee.ai/stable/hw/manuals/biases.html
// Os calores para a ca~mera SilkyEvCam pertencem a geração Gen3.1 VGA, assim os valores máximos e mínimo

const int bias_diff_default = 299; // Não alterar o valor do bias_diff, o default é 299.

int bias_diff_on_min        = bias_diff_default + 75; // O valor mínimo do bias_diff_on é bias_dif_default + 75.
int bias_diff_on_max        = bias_diff_default + 200; // O valor máximo do bias_diff_on é bias_dif_default + 200.

int bias_diff_off_min       = 100; // O valor mínimo do bias_diff_off é 100.
int bias_diff_off_max       = bias_diff_default - 65; // O valor máximo do bias_diff_off é bias_dif_default -65

int bias_fo_min             = 1250;
int bias_fo_max             = 1800;

int bias_hpf_min            = 900;
int bias_hpf_max            = 1800;

int bias_refr_min           = 1300;
int bias_refr_max           = 1800;

int bias_diff;
int bias_diff_on;
int bias_diff_off;
int bias_fo;
int bias_hpf;
int bias_refr;
};


// Parametros gerais auxiliares ao procedimento de calibração:
struct ParamsGlobais{

  std::string program_name= "calib_cam_1.0";
  std::string pattern_type= "CHESSBOARD";

  int cols= 9;
  int rows= 6;
  float square_dist= 0.05;
  bool refine_calibration= false;
  bool projeta_padrao= true;
};



// Esta struct é usada para guardar os argumentos passados para a função do Metavision SDK "calibration_recording": 
struct calibration_parameters {
    // 
    std::vector<std::string> args_data;

    // 
    std::vector<char*> args_ptrs;

    int argc= 0;
    char** argv= nullptr;
};