/*
* Author: Moacir Wendhausen.
* Date: 2025/11/20.
* Event camera: SilkyEvCam Centuri Arks (VGA).
* SDK: Metavision 4.6.2.
*/

// Define a macro exigida pelas versões recentes do Boost para manter o uso de placeholders globais (_1, _2).
// Isso silencia o aviso de depreciação gerado internamente pela inclusão do <boost/property_tree/json_parser.hpp>.
#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <metavision/sdk/base/utils/log.h>
#include <metavision/sdk/driver/camera.h>
#include <iostream>
#include <limits>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <stdio.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <nlohmann/json.hpp>

#include "blinking_pattern_focus.h"
#include "calibration_recording.h"
#include "matavision_mono_calibration.h"
#include "parameters.hpp"    



// Limpa terminal:
void limpaTela(){
    std::cout << "\033[2J\033[1;1H" << std::flush;
}




// This function calls the Metavision SDK adjust focus routine:
void ajustaFoco(){
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
bool testaConexaoEvcam(){
    limpaTela();

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

    std::cout<< "Camera conectada:\n" ;    
    std::cout<< "****************************\n";    
    std::cout<< " - Serial number: "<< current_config.serial_number << "\n";
    std::cout<< " - Firmware: "<< current_config.firmware_version << "\n";
    std::cout<< " - Fabricante: "<< current_config.integrator << "\n";
    std::cout<< "****************************\n";    

    return true;
}



void capturaFramesDeCalibracao(Bias &params_bias, ParamsGlobais &params_globais, FilesNames &files_names){
    calibration_parameters params;

    // Pattern name:
    params.args_data.push_back(params_globais.program_name);
    
    // Pattern Type:
    params.args_data.push_back("--pattern-type");
    
    //params.args_data.push_back("pattern_type");
    params.args_data.push_back(params_globais.pattern_type); 

    // Width (Number of inner corners of cols):
    params.args_data.push_back("--cols"); // Nome correto do parâmetro do SDK
    params.args_data.push_back(std::to_string(params_globais.cols));

    // Height (Number of the inner corners of rows)
    params.args_data.push_back("--rows"); // Nome correto do parâmetro do SDK
    params.args_data.push_back(std::to_string(params_globais.rows));

    // Name of raw data output file (raw data = images).
    // OUtput file name:
    params.args_data.push_back("-o"); // Nome correto do parâmetro do SDK
    params.args_data.push_back(files_names.output_file);
     
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
 
    // Calling SDK Metavision calibration recording imagens function: 
    int var_teste= calibration_recording(params.argc, params.argv, params_bias);
}



void geraParametrosIntrinsecos(ParamsGlobais &params_globais, FilesNames &files_names){
    calibration_parameters params;

    std::string program_name= params_globais.program_name;    
    std::string output_file= files_names.output_file; 
    bool refine_calib= params_globais.refine_calibration;
    
    // Creating data vectors (Strings).
    params.args_data.push_back(program_name);  

    // Pattern Type:
    params.args_data.push_back("-i");

    //params.args_data.push_back("pattern_type");
    params.args_data.push_back(output_file);

    if (refine_calib) {
        // Width (Number of inner corners of cols):
        params.args_data.push_back("-r"); // Nome correto do parâmetro do SDK
        params.args_data.push_back("REFINE_AND_SHOW_IMAGES");
    }

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

    // Calling SDK Metavision calibration to generate intrinsics parameters: 
    extract_intrinsics_parameters(params.argc, params.argv); 
}



// Show menu function:
void show_menu(){
    std::cout<< "\n";
    std::cout<< "\n";
    std::cout<< "********** Menu de Calibração **********\n";
    std::cout<< "\n";
    std::cout<< "  1-> Testar conexão com a SilkyEvCam\n";
    std::cout<< "  2-> Ajustar foco da lente \n";
    std::cout<< "  3-> Calibração - Aquisição de frames \n";
    std::cout<< "  4-> Calibração - Gerar parâmetros intrinsecos \n";
    std::cout<< "  5-> Ler biases do .json \n";
    std::cout<< "  6-> Ler parametros do .json \n" ;
    std::cout<< "\n";
    std::cout<< "  Q-> Sair"; 
    std::cout<< "\n";
    std::cout<< "****************************************\n";
    std::cout<< "  Digite a opção:\n";
}




bool loadBiasFromJson(Bias &val, const std::string filename){
    std::cout << "Lendo arquivo: " << filename << " ..." << std::endl;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return false;
    }    

    nlohmann::json data_json;
    // Parse completo ára varivael data_jason
    file >> data_json;    

    if (!data_json.contains("ll_biases_state") || !data_json["ll_biases_state"].contains("bias")){
        std::cout << "[Erro] Não foi possivel fazer o parsing do json! Verifique a integridade do arquivo: " << filename << "\n";
        return false;
    }

    // Parsing direto:
    val.bias_diff= data_json["ll_biases_state"]["bias"][0]["value"].get<int>();
    std::cout << "bias_diff= " << val.bias_diff << "\n";

    val.bias_diff_off= data_json["ll_biases_state"]["bias"][1]["value"].get<int>();
    std::cout << "bias_diff_off= " << val.bias_diff_off << "\n";

    val.bias_diff_on= data_json["ll_biases_state"]["bias"][2]["value"].get<int>();
    std::cout << "bias_diff_on= " << val.bias_diff_on << "\n";

    val.bias_fo= data_json["ll_biases_state"]["bias"][3]["value"].get<int>();
    std::cout << "bias_fo= " << val.bias_fo << "\n";

    val.bias_hpf= data_json["ll_biases_state"]["bias"][4]["value"].get<int>();
    std::cout << "bias_hpf= " << val.bias_hpf << "\n";

    val.bias_refr= data_json["ll_biases_state"]["bias"][6]["value"].get<int>();
    std::cout << "bias_refr= " << val.bias_refr << "\n";

    return true;
}



// Efetua o parssing do arquivo que contem os paramentors gerais e carrega na variavel struct parmas, definida em "parametros.hpp":
bool loadParamsFromJson(ParamsGlobais &params, FilesNames f_names, const std::string &filename) {
    
    std::cout << "Lendo arquivo: " << filename << " ..." << std::endl;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return false;
    }
    
    nlohmann::json data_json;
    // Parse completo ára varivael data_jason
    file >> data_json;

    // Parsing linha a linha do arquivo .json:
    params.program_name= data_json["program_name"].get<std::string>();
    params.pattern_type= data_json["pattern_type"].get<std::string>();
    std::cout << " - Padrao de calibração: " << params.pattern_type << "\n"; 

    params.cols= data_json["cols"].get<int>();
    std::cout << " - Nº colunas do padrão: " << params.cols << "\n"; 
    
    params.rows= data_json["rows"].get<int>();
    std::cout << " - Nº Linhas do padrão: " << params.rows << "\n"; 

    params.square_dist= data_json["square_dist"].get<double>();
    std::cout << " - Dimensão do quadrado: " << params.square_dist << "\n";    
    
    params.refine_calibration= data_json["refine_calibration"].get<bool>();
    params.projeta_padrao= data_json["projeta_padrao"].get<bool>();

    f_names.output_file= data_json["output_file"].get<std::string>();
    std::cout << " - Arquivo com matriz intrinseca: " << f_names.output_file << "\n";

    f_names.path_data= data_json["path_data"].get<std::string>();

    return true;
}




// O loop principal cham o menu IHM de usuario e em função da escolha cham a função a ser executada:
void loopPrincipal(Bias &params_bias, ParamsGlobais &params_globais, FilesNames &files_names){
    char my_choice;

    bool continue_run= true;
    while(continue_run){     
        limpaTela();

        // Chama função que exibe IHM com as opcoes de usuario: 
        show_menu();
        
        //Aguurada escolha do usuarao:
        std::cin>> my_choice;
        
        // ignora, descartar, os caracteres que ficaram na memória de entrada:
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        unsigned char choice= static_cast<unsigned char>(my_choice & 0xFF);
        switch (choice)
        {
            case '1':
            case 177:{
                limpaTela();
                // :
                if (!testaConexaoEvcam()){
                        std::cout<< "Camera não conectada!!! "<< std::endl;
                        std::cout<< "Digite qualquer tecla para retornar ao menu:" << std::endl;
                }
                std::cout << "\n";
                std::cin.get();
                std::cout << "Digite qualquer tecla \"Enter\" para retonar ao menu:\n";
                // Limpa buffer de teclado:
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;                               
            }

            case '2':
            case 178:{
                limpaTela();
                
                std::cout<< " Chamando função para ajustar o foco..." << std::endl;
                std::cout<< " Digite \"ESC\" para fechar o padrão piscante" << std::endl;
                // Chama a função que ajusta o foco:
                ajustaFoco();

                std::cout << "\n";
                std::cin.get();   
                std::cout << "Digite qualquer tecla \"Enter\" para retonar ao menu:\n";                   
                // Limpa buffer de teclado:
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;                               
            }
            
            case '3':
            case 179:{
                limpaTela();    
                
                // Chama função para efetuar a aquisição dos frames de eventos pára calibração:
                std::cout<< "Chamando função para captura de frame...." << std::endl;
                capturaFramesDeCalibracao(params_bias, params_globais, files_names);

                std::cout << "\n";
                std::cin.get();   
                std::cout << "Digite qualquer tecla \"Enter\" para retonar ao menu:\n";  ;      
                // Limpa buffer de teclado:
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;                               
            }
            
            case '4':
            case 180: {
                limpaTela();

                // Chama função para gerar os paramentros intrinsecos da camera e salvar em arquivo .json:
                geraParametrosIntrinsecos(params_globais, files_names);  

                std::cout << "\n";
                std::cin.get();   
                std::cout << "Digite qualquer tecla \"Enter\" para retonar ao menu:\n";       
                // Limpa buffer de teclado:
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;                               
            } 
            
            case '5':
            case 181:{
                limpaTela();

                // Chama função que carrega os os biases de arquivo json:
                if (!loadBiasFromJson(params_bias, files_names.bias_json)){
                    std::cout << " [Error] Nao foi possivel carregar o arquivo: " << files_names.bias_json << "\n";
                }                

                std::cout << "\n";
                std::cin.get();   
                std::cout << "Digite qualquer tecla \"Enter\" para retonar ao menu:\n";               
                // Limpa buffer de teclado:
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;
            }

            case '6':
            case 182:{
                limpaTela();

                // Chama função que carrega os parametros gerais de arquivo json:
                if (!loadParamsFromJson(params_globais, files_names, files_names.params_json)){
                    std::cout << " [Error] Nao foi possivel carregar o arquivo: " << files_names.params_json << "\n";
                }

                std::cout << "\n";
                std::cin.get();   
                std::cout << "Digite qualquer tecla \"Enter\" para retonar ao menu:\n";      
                // Limpa buffer de teclado:
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;                               
            }                

            case 27: // ESC
            case 'Q':
            case 'q':
                continue_run= false;
                break;  

            default:
            break;
        }
    }
}




int main(){
    Bias params_bias;
    ParamsGlobais params_globais;
    FilesNames files_names;

    // Limpa terminal:
    limpaTela();

    // Carrega os param,etros gerais usados no procedimentos de calibraçao definidos em "params.json":    
    if (!loadParamsFromJson(params_globais, files_names, files_names.params_json)){
        std::cout << " [Error] Nao foi possivel carregar o arquivo: " << files_names.params_json << "\n";
    }

    // Carrega os paramentros usados para configurar os "biases" da camera, definidos em "settings.json":
    if (!loadBiasFromJson(params_bias, files_names.bias_json)){
        std::cout << " [Error] Nao foi possivel carregar o arquivo: " << files_names.bias_json << "\n";
    }

    
    // Executa o loop principal da calibração: 
    loopPrincipal(params_bias, params_globais, files_names);

    return 0;
}