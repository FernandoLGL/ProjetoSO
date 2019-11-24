#include <array>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// são vectors e não arrays por causa da função clear
static std::vector<float> s_bufferGrava;
static std::vector<float> s_bufferSerial;
static std::queue<float> s_queueTemp;

float mediaAritmetica(const std::vector<float> &valores) {
  float resultado{};
  for (const auto &valor : valores)
    resultado += valor;
  return resultado / valores.size();
}

// lê periodicamente (a cada 1000 ms) o sensor de temperatura do processador
// Tiva C, calcula a média aritmética e envia para as tasks Grava e Serial por
// meio de 2 filas (queues).
void temperatura();

// A task Grava armazena os valores lidos da fila em um Buffer de 10 posições
// até que seja totalmente preenchido. Após o preenchimento do Buffer, os dados
// são transferidos para a memória Grava do microcontrolador e o Buffer é
// esvaziado, reiniciando seu processo de preenchimento.
void grava(const std::queue<float> &mediaAritmetica);

// A task Serial funciona de forma similar à task Grava. Ela lê os dados
// enviados pela task Temperatura por meio de outra fila e envia por meio da
// UART os 10 últimos valores lidos.
void serial(const std::queue<float> &mediaAritmetica);

// A comunicação e coordenação entre as tarefas são executadas por meio das duas
// filas (Temperatura ⇒ Grava e Temperatura ⇒ Serial) e do mutex.

int main() {
  s_bufferGrava.reserve(10);
  s_bufferSerial.reserve(10);

  std::thread temperatura(temperatura);

  temperatura.join();

  std::thread grava(grava);
  std::thread serial(serial);
}

void temperatura() {

  using namespace std::literals::chrono_literals;

  // esperar um segundo pra ler
  std::this_thread::sleep_for(1s);

  // Vai armazenar os valoes no vetor, mandar o vetor pra calcular a média pra
  // daí dar queue em s_queueTemp pra mandar pra Serial e Grava
}

void grava(const std::queue<float> &mediaAritmetica) {
  if (s_bufferGrava.size() == s_bufferGrava.maxSize()) {
    for (valor : s_bufferGrava) {
      // envia aqui os dados pra memória Grava do microcontrolador
    }
    // esvazia o conteúdo
    s_bufferGrava.clear();
  }
}

void serial(const std::queue<float> &mediaAritmetica) {}
