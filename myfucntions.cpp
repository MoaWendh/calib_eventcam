

#include "parameters.hpp"

#include "myfucntions.hpp"



// Faz a leitura dos biases na camera de eventos e exibe na tela os valores lidos:
void readBiasesCam(Metavision::Camera &cam){
    // Acessa a Facility de Biases de baixo nível:
    auto *i_ll_biases = cam.get_device().get_facility<Metavision::I_LL_Biases>();
    
    // Testa se o acesso foi liberado:
    if (!i_ll_biases) {
        std::cerr << "[Erro] Nao foi possivel acessar a interface de Biases do hardware!" << std::endl;
        return;
    }

    std::cout << "\nValore lidos biases lido da eventcam: \n" ;
    // Captura os biases da camera:

    int val;
    try {
        // Efetua a leitura dos biases da camera. Este processo é "on-the-fly", sem precisar de cam.stop()
        val= i_ll_biases->get("bias_diff_on");
        std::cout << " - bias_diff_on: " << val << "\n";

        val= i_ll_biases->get("bias_diff_off");
        std::cout << " - bias_diff_off: " << val << "\n";

        val= i_ll_biases->get("bias_fo");
        std::cout << " - bias_fo: " << val << "\n";

        val= i_ll_biases->get("bias_hpf");
        std::cout << " - bias_hpf: " << val << "\n";

        val= i_ll_biases->get("bias_refr");
        std::cout << " - bias_refr: " << val << "\n\n";

    } catch (const std::exception &e) {
        std::cerr << "[Erro] Não foi possível ler os biases na câmera!!! " << e.what() << std::endl;
        return;
    }  
}


bool writeBiasesCam(Metavision::Camera &cam, Bias &params){
    std::string serial= cam.get_camera_configuration().serial_number;
    std::string fabricante= cam.get_camera_configuration().integrator;


    // Acessa a Facility de Biases de baixo nível:
    auto *i_ll_biases= cam.get_device().get_facility<Metavision::I_LL_Biases>();
    
    // Testa se o acesso foi liberado:
    if (!i_ll_biases) {
        std::cerr << "[Erro] Nao foi possivel acessar a interface de Biases do hardware!" << std::endl;
        return 0;
    }

    // Atualzia os biases na câmera:
    try {
        // Atuazia os valores um por um diretamente no registrador do sensor. Este processo é "on-the-fly", sem precisar de cam.stop()
        i_ll_biases->set("bias_diff_on", params.bias_diff_on);
        i_ll_biases->set("bias_diff_off", params.bias_diff_off);
        i_ll_biases->set("bias_fo", params.bias_fo);
        i_ll_biases->set("bias_hpf", params.bias_hpf);
        i_ll_biases->set("bias_refr", params.bias_refr);
    } catch (const std::exception &e) {
        std::cerr << "[Erro] Não foi possível gravar biases na câmera!!! " << e.what() << std::endl;
        return false;
    } 
    
    std::cout << "Câmera " << fabricante << ":....... NºSérie " << serial << "\n";
    std::cout << "Câmera " << fabricante << ":....... " << "Biases atualizados\n";
    return true;    
}

