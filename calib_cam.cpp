/*
* Author: Moacir Wendhausen.
* Date: 2025/11/20.
* Event camera: SilkyEvCam Centuri Arks (VGA).
* SDK: Metavision 4.6.2.
*/

#include    <metavision/sdk/base/utils/log.h>
#include    <metavision/sdk/driver/camera.h>
#include    <iostream>
#include    <limits>
#include    <cstdlib>
#include    <vector>
#include    <string>
#include    <boost/property_tree/ptree.hpp>
#include    <boost/property_tree/json_parser.hpp>
#include    <nlohmann/json.hpp>
#include    <fstream>

#include    "blinking_pattern_focus.h"
#include    "calibration_recording.h"


// This struct maintaains data in the memori for "argc" and "argv" to calibration data colection:
struct calibration_parameters {
    // Strings data container:
    std::vector<std::string> args_data; 
    
    // Vector dara Container to pointers (argv)
    std::vector<char*> args_ptrs;
    
    // 
    int argc = 0;
    char** argv = nullptr;
};



// Clean screen function:
void clear_screem(){
    //std::cout << "\033[2J\033[1;1H";
    //system("clear");
    std::cout << "\033[2J\033[1;1H" << std::flush;
}


// Show menu function:
char show_menu(){
    char choice;
    clear_screem();

    std::cout<< "" <<std::endl;
    std::cout<< "" <<std::endl;
    std::cout<< "********** Menu de Calibração *****" <<std::endl;
    std::cout<< "" <<std::endl;
    std::cout<< "  (a) Testa conexão com a SilkyEvCam" <<std::endl;
    std::cout<< "  (b) Ajustar foco da lente" <<std::endl;
    std::cout<< "  (c) Calibração - Aquisição de frames" <<std::endl;
    std::cout<< "  (d) Calibração - Gerar parâmetros" <<std::endl;
    std::cout<< "  (q) Sair" <<std::endl;
    std::cout<< "" <<std::endl;
    std::cout<< "**********************************" <<std::endl;
    std::cout<< "   Escolha uma das opções acima." <<std::endl;
    
    //Wait choice:
    std::cin>> choice;

    // lear keyboard buffer:
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Call celar screem fuction:
    clear_screem();

    return choice; 
}


// This function calls the Metavision SDK adjust focus routine:
void adjust_focus(){
    // It defines the path file name (.jpg) that defines the star pattern.
    const std::string full_path_pattern_figure = "../blink-pattern.jpg";
    
    // Definition of the arguments:
    char arg0[] = "blik-pattern"; // Program name.
    char arg1[] = "--pattern-image-path";  // Metavision SDK parameter.
    int len= full_path_pattern_figure.length();
    char arg2[len+1];
    std:strcpy(arg2, full_path_pattern_figure.c_str());

    // Crie o array de argumentos para passar à rotina que ira piscar o padrão de estrela
    // para ajuste do foco:
    char* argumentos[] = {
        arg0,
        arg1,
        arg2,
    };
    
    // arg_c contém a quantidade de argumentos. 
    // arg_c é passdo como parametro para a função blinking_pattern_focus(). 
    int argc_aux = std::size(argumentos);
    
    // Chama a função que executa a rotian de ajuste de foco como o padrão piscante
    // Esta rotina é do SDK Metavision. Ela recebe 2 parâmetros.
    int var_teste= blinking_pattern_focus(argc_aux, &argumentos[0]);
}



// APenas verifica se a camera está conectada na USB
bool test_conection_evcam(){
    clear_screem();

    std::cout<<' '<<std::endl;
    std::cout<<' '<<std::endl;

    // Instancia um objeto camera tipo SilkyEvCam:
    Metavision::Camera camera;

    try {
        camera = Metavision::Camera::from_first_available();
    } catch (const Metavision::CameraException &e) {
        MV_LOG_ERROR() << e.what();
        return false;
    }
    
    // Captura a configuração da câmera:
    Metavision::CameraConfiguration current_config=  camera.get_camera_configuration();

    std::cout<< "Camera conectada ok!!" << std::endl;    
    std::cout<< "****************************" << std::endl;    
    std::cout<< " - Serial number: "<< current_config.serial_number << std::endl;
    std::cout<< " - Firmware: "<< current_config.firmware_version << std::endl;
    std::cout<< " - Fabricante: "<< current_config.integrator << std::endl;
    std::cout<< "****************************" << std::endl;    
    std::cout<< "Digite qualquer tecla para retornar ao menu:" << std::endl;
    std::cin.get();
    return true;
}


// Reading the parameters from "calib_params.json":
calibration_parameters load_calib_params_from_json(const std::string& json_file) {
    calibration_parameters params;
    namespace pt = boost::property_tree;
    pt::ptree root;
    nlohmann::json json_var;


    // Trying to open .json file:
    try {
        std::ifstream file_json(json_file);
        file_json >> json_var;
    } catch (const std::exception &e) {
        std::cerr << "[Erro] Falha ao tentar abrir aqruivo:" << json_file << "': " << e.what() << std::endl;
        return params; // Return an ampty struct (argc = 0).
    }
    
    // Recovering data from json object:
    std::string program_name= json_var["program_name"].get<std::string>(); 
    std::string pattern_type= json_var["pattern_type"].get<std::string>();
    int cols= json_var["cols"].get<int>();
    int rows= json_var["rows"].get<int>();
    std::string output_file= json_var["output_file"].get<std::string>();
    float square_dist= json_var["square_dist"].get<float>();

    // Creating data vectors (Strings).    
    // Pattern Type:
    params.args_data.push_back("--pattern-type");
    params.args_data.push_back("pattern_type");

    // Width (Number of inner corners of cols):
    params.args_data.push_back("--cols"); // Nome correto do parâmetro do SDK
    params.args_data.push_back(std::to_string(cols));

    // Height (Number of the inner corners of rows)
    params.args_data.push_back("--rows"); // Nome correto do parâmetro do SDK
    params.args_data.push_back(std::to_string(rows));

    // Name of raw data output file (raw data = images).
    // OUtput file name:
    params.args_data.push_back("-o"); // Nome correto do parâmetro do SDK
    params.args_data.push_back(output_file);
     
   
    // Creating pointer argv:
    params.args_ptrs.reserve(params.args_data.size());

    for (auto &str : params.args_data) {
        // const_cast remove o 'const' do c_str(), pois argv é char**
        params.args_ptrs.push_back(const_cast<char*>(str.c_str()));
    }
    params.args_ptrs.push_back(nullptr); // Padrão C/C++: argv deve terminar com NULL

    // Setting arc and argv variables:
    params.argc = params.args_ptrs.size() - 1; // Desconta o nullptr
    params.argv = params.args_ptrs.data();

    return params;
}



void calibration_acquire_frames(){
    // Defining .json name file:
    std::string json_file= "../calib_params.json";

    // Recovering params from .json file:
    calibration_parameters param= load_calib_params_from_json(json_file);

    // Calling SDK Metavision calibration recording imagens function: 
    int var_teste= calibration_recording(param.argc, param.argv);

    /*   
    // Definition of the arguments:
    char arg0[] = "acquire_pattern"; // Program name.
    char arg1[] = "--pattern-type";  // Metavision SDK parameter.
    char arg2[] = "CHESSBOARD";  // Metavision SDK parameter.
    char arg3[] = "--cols";  // Metavision SDK parameter.
    char arg4[] = "9";  // Metavision SDK parameter.
    char arg5[] = "--rows";  // Metavision SDK parameter.
    char arg6[] = "6";  // Metavision SDK parameter.
    char arg7[] = "-o";  // Metavision SDK parameter.
    char arg8[] = "calibracao_teste.raw";  // Metavision SDK parameter.

    // Crie o array de argumentos para passar à rotina que ira piscar o padrão de estrela
    // para ajuste do foco:
    char* argumentos[] = {
        arg0,
        arg1,
        arg2,
        arg3,
        arg4,
        arg5,
        arg6,
        arg7,
        arg8,
    };
    
    // arg_c contém a quantidade de argumentos. 
    // arg_c é passdo como parametro para a função blinking_pattern_focus(). 
    int argc_aux = std::size(argumentos);
    
    
    // Chama a função que executa a rotian de ajuste de foco como o padrão piscante
    // Esta rotina é do SDK Metavision. Ela recebe 2 parâmetros.
    //int var_teste= calibration_recording(argc_aux, &argumentos[0]);
    int var_teste= calibration_recording(argc_aux, argumentos);
*/
}


void calibration_generate_parameters(){
    
}


// This function selected the routine according to the content of the choice variable:
bool trata_menu(char choice){
    
    switch (choice)
    {
        case 'a':
        case 'A':
           // :
           if (!test_conection_evcam()){
                std::cout<< "Camera não conectada!!! "<< std::endl;
                std::cout<< "Digite qualquer tecla para retornar ao menu:" << std::endl;
                std::cin.get();
           }
            return true;
            break;

        case 'b':
        case 'B':
            adjust_focus();
            return true;
            break;
         
        case 'c':
        case 'C':
            calibration_acquire_frames();
            return true;
            break;
        
        case 'D':
        case 'd': 
            calibration_generate_parameters();               
            return true;
            break;
        
        case 'q':
        case 'Q':
            return false;
            break;

        default:
            return true;
    }
}



int main(){
    char my_choice;
    bool continue_run;

    continue_run= true;

    clear_screem();

    while(continue_run){
        // Calling function to show IHM.
        // It returns a charr variable to verify the choice: 
        my_choice= show_menu();

        // Calling function to test the choice.
        // Testing the variable "my_choice": 
        continue_run= trata_menu(my_choice);
    }
    
    
    return 0;
}