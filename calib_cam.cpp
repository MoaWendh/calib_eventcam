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

#include    "blinking_pattern_focus.h"
#include    "calibration_recording.h"


// This struct maintaains data in the memori for argc/argv to calibration data colection:
struct calib_parameters {
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

int tst(int argc, char *argv[]) {

    return 0;    

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
bool testa_conexão_evcam(){
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



calib_parameters load_calib_params_from_json(const std::string& json_file) {
    calib_parameters params;
    namespace pt = boost::property_tree;
    pt::ptree root;

    // 1. Try to read .json file:
    try {
        pt::read_json(json_file, root);
    } catch (const std::exception &e) {
        std::cerr << "[Erro] Falha ao ler '" << json_file << "': " << e.what() << std::endl;
        return params; // Retorna struct vazia (argc = 0)
    }

    // 2. Preenchee o vetor de dados (Strings)
    // IMPORTANTE: Preencher todas as strings PRIMEIRO para garantir que
    // elas não mudem de endereço de memória depois.
    
    // Nome do programa:
    params.args_data.push_back(root.get<std::string>("program_name", "recorder_app"));

    // Pattern Type
    params.args_data.push_back("--pattern-type");
    params.args_data.push_back(root.get<std::string>("pattern_type", "chessboard"));

    // Width (Cols)
    params.args_data.push_back("--cols"); // Nome correto do parâmetro do SDK
    params.args_data.push_back(root.get<std::string>("cols", "9"));

    // Height (Rows)
    params.args_data.push_back("--rows"); // Nome correto do parâmetro do SDK
    params.args_data.push_back(root.get<std::string>("rows", "6"));

    // Output File
    params.args_data.push_back("-o"); // Flag curta (um traço)
    params.args_data.push_back(root.get<std::string>("output_file", "calibracao_teste.raw"));

    /*
    // Opcional: Square Dist (se estiver no JSON)
    if (root.get_child_optional("square_dist")) {
        params.args_data.push_back("--square-dist");
        params.args_data.push_back(root.get<std::string>("square_dist"));
    }*/

    // 3. Preencher o vetor de ponteiros (argv)
    // Agora que as strings estão fixas na memória, criamos os ponteiros para elas.
    params.args_ptrs.reserve(params.args_data.size() + 1);

    for (auto &str : params.args_data) {
        // const_cast remove o 'const' do c_str(), pois argv é char**
        params.args_ptrs.push_back(const_cast<char*>(str.c_str()));
    }
    params.args_ptrs.push_back(nullptr); // Padrão C/C++: argv deve terminar com NULL

    // 4. Configurar as variáveis de conveniência
    params.argc = params.args_ptrs.size() - 1; // Desconta o nullptr
    params.argv = params.args_ptrs.data();

    return params;
}



void calibration_acquire_frames(){
    std::string json_file= "/home/moa/projects/c++/calib_evcam/calib_params.json";

    calib_parameters param= load_calib_params_from_json(json_file);

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


bool trata_menu(char choice){
    
    switch (choice)
    {
        case 'a':
        case 'A':
           // :
           if (!testa_conexão_evcam()){
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

    //std::cout<<"Passou Aqui!!"<<std::endl;

    while(continue_run){
        my_choice= show_menu();

        continue_run= trata_menu(my_choice);
    }
    
    
    return 0;
}