#define PLAYER_WAIT_TIME 2000 // O tempo permitido entre as pressões de botão - 2s

byte sequence[100];           // Armazenamento da sequência de luzes
byte curLen = 0;              // Comprimento atual da sequência
byte inputCount = 0;          // O número de vezes que o jogador pressionou um botão (correto) em uma determinada rodada
byte lastInput = 0;           // Última entrada do jogador
byte expRd = 0;               // O LED que deve ser iluminado pelo jogador
bool btnDwn = false;          // Usado para verificar se um botão está pressionado
bool wait = false;            // O programa está esperando o usuário pressionar um botão
bool resetFlag = false;       // Usado para indicar ao programa que o jogador perdeu

byte pins[] = {26, 13, 27, 25}; // Pinos de entrada de botão e pinos de saída de LED - alterar esses valores fará com que os botões sejam conectados aos pinos correspondentes aos valores
                                // O número de elementos deve corresponder a noPins abaixo*/
byte noPins = 4;              // Número de botões/LEDs (enquanto trabalhava nisso, usei 4 LEDs)
                              // Você pode tornar o jogo mais difícil adicionando uma combinação adicional de LED/botão/resistores.
byte resetButton = 15;        //Porta correspondente ao botão de Reset

long inputTime = 0;           // Variável temporizador para o atraso entre as entradas do usuário

void setup() {
  pinMode(resetButton, INPUT_PULLDOWN);
  delay(3000); // Isso é para dar tempo de respirar após conectar o arduino
  Serial.begin(9600); // Inicie o monitor Serial. Isso pode ser removido, desde que remova todas as referências ao Serial abaixo.
  Reset();
}

// Define todas as portas como INPUT ou OUTPUT com base no valor de 'dir'
void setPinDirection(byte dir){
  for(byte i = 0; i < noPins; i++){
    pinMode(pins[i], dir);
  }
}

// envia o mesmo valor para todas as portas LED
void writeAllPins(byte val){
  for(byte i = 0; i < noPins; i++){
    digitalWrite(pins[i], val);
  }
}

// Pisca todos os LEDs juntos
// freq é a velocidade do piscar - número pequeno -> rápido | número grande -> lento
void flash(short freq){
  setPinDirection(OUTPUT); // Ativando os LEDs agora
  for(int i = 0; i < 5; i++){
    writeAllPins(HIGH);
    delay(freq);
    writeAllPins(LOW);
    delay(freq);
  }
}

// Esta função redefine todas as variáveis do jogo para seus valores padrão.
void Reset(){
  flash(500);
  curLen = 0;
  inputCount = 0;
  lastInput = 0;
  expRd = 0;
  btnDwn = false;
  wait = false;
  resetFlag = false;
}

// Usuário perdeu
void Lose(){
  flash(50);
}

// O arduino mostra ao usuário o que deve ser memorizado
// Também chamado após a perda para mostrar qual foi a última sequência
///
void playSequence(){
// Percorre a sequência armazenada e acende os LEDs apropriados por vez
  for(int i = 0; i < curLen; i++){
    Serial.print("Seq: ");
    Serial.print(i);
    Serial.print("Pin: ");
    Serial.println(sequence[i]);
    digitalWrite(sequence[i], HIGH);
    delay(500);
    digitalWrite(sequence[i], LOW);
    delay(250);
  }
}

///
/// Os eventos que ocorrem após a perda
///
void DoLoseProcess(){
  Lose(); // Flash em todos os LEDs rapidamente (consulte a função Lose)
  delay(1000);
  playSequence(); // Mostra ao usuário a última sequência - Para que você possa se lembrar da sua melhor pontuação
  delay(1000);
  Reset(); // Redefinir tudo para um novo jogo
}

///
/// Onde a mágica acontece
///
void loop() {
  if(!wait){
  ////
  // Turno do Arduino //
  ////
  setPinDirection(OUTPUT); // Estamos usando os LEDs
        
  randomSeed(analogRead(A0)); // Define a semente aleatória baseada na leitura analógica da porta A0 - https://www.arduino.cc/en/Reference/RandomSeed
  sequence[curLen] = pins[random(0, noPins)]; // Armazena um novo valor aleatório na próxima posição na sequência - https://www.arduino.cc/en/Reference/random
  curLen++; // Define o novo comprimento atual da sequência

  playSequence(); // Mostra a sequência para o jogador

  wait = true; // Define "wait" como verdadeiro, pois agora será a vez do jogador
  inputTime = millis(); // Armazena o tempo para medir o tempo de resposta do jogador
  } else {
  ////
  // Vez do jogador //
  ////
  setPinDirection(INPUT); // Estamos usando os botões

  if (millis() - inputTime > PLAYER_WAIT_TIME) { // Se o jogador demorar mais do que o tempo permitido,
    DoLoseProcess(); // ele perde
    return;
  }

  if (!btnDwn) {
    expRd = sequence[inputCount]; // Encontra o valor que esperamos do jogador
    Serial.print("Esperado: "); // Saída do Monitor Serial
    Serial.println(expRd); // Saída do Monitor Serial

    for (int i = 0; i < noPins; i++) { // Loop através de todos os pinos
      if (pins[i] == expRd)
        continue; // Ignora o pino correto
      if (digitalRead(pins[i]) == HIGH) { // O botão está pressionado
        lastInput = pins[i];
        resetFlag = true; // Define o resetFlag - isso significa que você perdeu
        btnDwn = true; // Isso impedirá que o programa faça a mesma coisa repetidamente
        Serial.print("Lido: "); // Saída do Monitor Serial
        Serial.println(lastInput); // Saída do Monitor Serial
      }
    }
  }

  while (digitalRead(resetButton) == HIGH) {
    Serial.print("Lido: "); // Saída do Monitor Serial
    Serial.println(resetButton); // Saída do Monitor Serial
    Reset();  // O botão Reset foi pressionado e tudo recomeça com uma nova sequência aleatória
  }

  if (digitalRead(expRd) == 1 && !btnDwn) // O jogador pressionou o botão correto
  {
    inputTime = millis(); //
    lastInput = expRd;
    inputCount++; // O usuário pressionou um botão (correto) novamente
    btnDwn = true; // Isso impedirá que o programa faça a mesma coisa repetidamente
    Serial.print("Lido: "); // Saída do Monitor Serial
    Serial.println(lastInput); // Saída do Monitor Serial
  }
  else {
    if (btnDwn && digitalRead(lastInput) == LOW) { // Verifica se o jogador soltou o botão
      btnDwn = false;
      delay(20);
      if (resetFlag) { // Se isso foi definido como verdadeiro acima, você perdeu
        DoLoseProcess(); // É realizada a sequência de eventos de perda
      }
      else {
        if (inputCount == curLen) { // O jogador terminou de repetir a sequência?
          wait = false; // Se sim, isso fará com que a próxima jogada seja a do programa
          inputCount = 0; // Zerar o número de vezes que o jogador pressionou um botão
          delay(1500); // Aguardar 1,5 segundos antes de continuar para a próxima rodada
          }
      }
    }
  }
  }
}