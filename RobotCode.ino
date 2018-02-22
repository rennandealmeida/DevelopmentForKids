/*14/02/2016 (iniciado) - UNIFIEO - Trabalho de Conclusão de Curso  
  
 */

 /*Controles
  w = avançar
  s = recuar
  a = esquerda
  d = direita
  b = freio
  m = mede distância
  r = red_led (acende) -> t (apaga)
  y = yellow_led (acende) -> u (apaga)
  g = green_led (acende) -> h (apaga)
  
  obs: caso nao insira nada acima o robo fica no neutro
 */

#include <Ultrasonic.h> // Carrega biblioteca Ultrasonic (responsavel pelo sensor ultrasonico)

//definindo pinos para enviar e receber sinal ultraSonico
#define PINO_TRIGGER 3 //envia sinal
#define PINO_ECHO 2 //recebe sinal

// Calibragem do motor
#define DELAY_PASSO_FRENTE 2000
#define DELAY_VIRA_ESQUERDA 1720
#define DELAY_VIRA_DIREITA 1650

//Definindo pinos Leds
#define PIN_LED_VERMELHO 11
#define PIN_LED_VERDE 13
#define PIN_LED_AMARELO 12
#define PIN_LED_ALERT_VERMELHO 8
#define PIN_LED_ALERT_VERDE 10
#define PIN_LED_ALERT_AMARELO 9

//Definindo pinos Arduinos ligado a entrada da Ponte H
#define IN1 4 //fio vermelho ponte H (IN1)
#define IN2 5 //fio amarelo ponte H  (IN2)
#define IN3 6 //fio verde ponte H    (IN3)
#define IN4 7 //fio azul ponte H     (IN4)

Ultrasonic ultrasonic (PINO_TRIGGER, PINO_ECHO);

// Registradores
int instructionPoint = 0;
int bitComparator = 0;
int counter = 0;

//Definindo variavel que recebera dados de entrada enviados pelo bluetooth
char input;

void setup() {
  //inicia comunicação serial modulo Bluetooth
  Serial.begin(9600);
  
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);

  pinMode(PIN_LED_VERMELHO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMARELO, OUTPUT);
  pinMode(PIN_LED_ALERT_AMARELO, OUTPUT);
  pinMode(PIN_LED_ALERT_VERDE, OUTPUT);
  pinMode(PIN_LED_ALERT_VERMELHO, OUTPUT);
}

void loop() {
  if(Serial.available() > 0) {
    String tamanho = Serial.readStringUntil('-');
  
    int sizeCommands = tamanho.toInt();
    String commands[sizeCommands];
    
    int loadIndex = 0;
    String command = "";
    
    // Lê todos os caracteres recebidos
     while(Serial.available() > 0) {
      
      // A variável input recebe o valor na Serial (para imprimir no serial monitor)
      input = Serial.read();
      
      if(input == '|') {
        commands[loadIndex] = command;
        loadIndex = loadIndex + 1;
        command = "";
      } else {
        // Concatena os commands
        command.concat(input);
      }
    }
    commands[loadIndex] = command;
    loadIndex = loadIndex + 1;

    Serial.flush();
    if(command.length() > 0) {
      digitalWrite(PIN_LED_ALERT_VERDE, HIGH);
      interpretaComando(sizeCommands, commands);
      digitalWrite(PIN_LED_ALERT_VERDE, LOW);
    }
  }
  
  delay(1000);
}

void interpretaComando(int sizeCommands, String commands[]) {
  // Registradores
  instructionPoint = 0;
  bitComparator = 0;
  counter = 0;
  int finished = 0;
  
  String command;
  char instruction;
  String arg;

  resetaEstado();
  
  while(instructionPoint < sizeCommands) {
    command = commands[instructionPoint];
    ////Serial.print("Cmd[");
    ////Serial.print(instructionPoint);
    ////Serial.print("]: ");
    ////Serial.println(command);
    
    if(command.length() > 0) {
      instruction = command.charAt(0);
      switch(instruction) {
        // Instruções das LEDs
        case 'g':
          acendeLedVerde();
          break;
        case 'y':
          acendeLedAmarelo();
          break;
        case 'r':
          acendeLedVermelho();
          break;
        case 'h':
          apagaLedVerde();
          break;
        case 'u':
          apagaLedAmarelo();
          break;
        case 't':
          apagaLedVermelho();
          break;


        // Instruções de Movimentos
        case 'w':
          arg = extraiArgumento(command);
          avanca(arg.toInt());
          break;
        case 's':
          arg = extraiArgumento(command);
          recua(arg.toInt());
          break;
        case 'a':
          viraEsquerda();
          break;
        case 'd':
          viraDireita();
          break;

        // Saltos
        case 'j': // salto incondicional
          arg = extraiArgumento(command);
          instructionPoint = arg.toInt();
          continue;
          
        case 'z': // salto condicional
          if(bitComparator == 0) {
            arg = extraiArgumento(command);
            instructionPoint = arg.toInt();
            continue;
          }
          break;

        // Instruções
        case 'c': // Comparação
          comparaValores(command);
          break;
          
        case 'i': // Inicialização do contador
          arg = extraiArgumento(command);
          counter = arg.toInt();
          break;
          
        case 'p': // Incremento do contador
          counter = counter + 1;
          break;

        case '$':
          finished = 1;
          //Serial.println("Fim da interpretacao");
          //return;
      }
    }
    instructionPoint++;
    delay(500);
  }
  if(finished == 1) {
    Serial.write("0\r\n");
  } else {
    Serial.write("1\r\n");
  }
  Serial.flush();
  delay(1000);
  //Serial.println("Fim da interpretacao");
}

void comparaValores(String command) {
  int indexOperator = indiceOperador(command);
  int endIndexOperator = indexOperator;
  
  // Se o próximo char for ainda parte do operador
  if(command.charAt(indexOperator + 1) == '=') {
    endIndexOperator = endIndexOperator + 1;
  }
  // Extrai os operandos
  String firstOp = command.substring(1, indexOperator);
  String secondOp = command.substring(endIndexOperator + 1);
  float firstValue = 0;
  float secondValue = 0;
  
  // Extrai os valores para os operandos
  if(firstOp == "m")
    firstValue = medeDistancia();
  else if(firstOp == "c")
    firstValue = counter;
  else
    firstValue = firstOp.toFloat();
  
  // Extrai os valores para os operandos
  if(secondOp == "m")
    secondValue = medeDistancia();
  else if(secondOp == "c")
    secondValue = counter;
  else
    secondValue = secondOp.toFloat();

  // Efetua a comparação
  if(command.charAt(indexOperator) == '<' && command.charAt(endIndexOperator) == '=') {
    if(firstValue <= secondValue)
      bitComparator = 1;
    else
      bitComparator = 0;
  } else if(command.charAt(indexOperator) == '>' && command.charAt(endIndexOperator) == '=') {
    if(firstValue >= secondValue)
      bitComparator = 1;
    else
      bitComparator = 0;
  } else if(command.charAt(indexOperator) == '<') {
    if(firstValue < secondValue)
      bitComparator = 1;
    else
      bitComparator = 0;
  } else if(command.charAt(indexOperator) == '>') {
    if(firstValue > secondValue)
      bitComparator = 1;
    else
      bitComparator = 0;
  } else if(command.charAt(indexOperator) == '!' && command.charAt(endIndexOperator) == '=') {
    if(firstValue != secondValue)
      bitComparator = 1;
    else
      bitComparator = 0;
  } else if(command.charAt(indexOperator) == '=' && command.charAt(endIndexOperator) == '=') {
    if(firstValue == secondValue)
      bitComparator = 1;
    else
      bitComparator = 0;
  }
  //Serial.print(firstValue);
  //Serial.print(command.charAt(indexOperator));
  if(command.charAt(indexOperator) == '=') {
    //Serial.print("=");
  }
  //Serial.print(secondValue);
  //Serial.print(" = ");
  //Serial.println(bitComparator);
}

int indiceOperador(String comando) {
  int index = -1;

  index = comando.indexOf('<');
  if(index > 0)
    return index;
    
  index = comando.indexOf('>');
  if(index > 0)
    return index;
    
  index = comando.indexOf('!');
  if(index > 0)
    return index;
    
  index = comando.indexOf('=');
  if(index > 0)
    return index;
}

String extraiArgumento(String comando) {
  return comando.substring(1);
}

float medeDistancia() {
  //Serial.println("Mede distancia");
  //variaveis referentes ao sensor ultrasonico
  float cmMsec,inMsec;

  //lê os dados do sensor, com o tempo de retorno do sinal
  long microsec = ultrasonic.timing();
  
  //Calcula a distancia e converte para centimetros
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);

  return cmMsec;
}

void resetaEstado() {
  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_AMARELO, LOW);
  digitalWrite(PIN_LED_VERMELHO, LOW);
  digitalWrite(PIN_LED_ALERT_VERDE, LOW);
  digitalWrite(PIN_LED_ALERT_AMARELO, LOW);
  digitalWrite(PIN_LED_ALERT_VERMELHO, LOW);
  neutro();
}

void avanca(int nPassos) {
  float cmMsec;
  //verifica se a distancia do objeto é maior que 20 cm
  for(int i = 0; i < nPassos; i++) {
    digitalWrite(PIN_LED_ALERT_AMARELO, LOW);
    cmMsec = medeDistancia();
    if (cmMsec < 20) {
      digitalWrite(PIN_LED_ALERT_AMARELO, HIGH);
      freia();
      return;
    }
    //Serial.println("Robo Avançando");
    //Motores A e B no sentido Horario
    //referente ao motor A
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,HIGH);
    digitalWrite(IN4,LOW);
    delay(DELAY_PASSO_FRENTE);
    neutro();
    delay(500);
  }
}

void recua(int nPassos) {
  for(int i = 0; i < nPassos; i++) {
    //Serial.println("Robo Recuando");  
    
    //Motores A e B no sentido anti-Horario
    //referente ao motor A
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    delay(DELAY_PASSO_FRENTE);
    neutro();
    delay(500);
  }
}

void viraEsquerda() {
  //Serial.println("Robo virando para esquerda");  
  //referente ao motor B
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(DELAY_VIRA_ESQUERDA);
  neutro();
}

void viraDireita() {
  //Serial.println("Robo virando para direita");  
  //referente ao motor A
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  delay(DELAY_VIRA_DIREITA);
  neutro();
}

void freia() {
  //Serial.println("Robo freiando");  
  //Motor A e Motor B parado
  //referente ao motor A
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  //referente ao motor B
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  delay(500);
}

void neutro() {
  //Motor A e Motor B ponto Neutro
  //referente ao motor A
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  //referente ao motor B
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void acendeLedVermelho() {
  //Serial.println("Acender LED vermelho");
  digitalWrite(PIN_LED_VERMELHO, HIGH);  
}

void apagaLedVermelho() {
  //Serial.println("Apagando LED vermelho");  
  digitalWrite(PIN_LED_VERMELHO, LOW);  
}

void acendeLedAmarelo() {
  //Serial.println("Acender LED amarelo");  
  digitalWrite(PIN_LED_AMARELO, HIGH);  
}

void apagaLedAmarelo() {
  //Serial.println("Apagando LED amarelo");  
  digitalWrite(PIN_LED_AMARELO, LOW);  
}

void acendeLedVerde() {
  //Serial.println("Acender LED verde");  
  digitalWrite(PIN_LED_VERDE, HIGH);  
}

void apagaLedVerde() {
  //Serial.println("Apagando LED verde");  
  digitalWrite(PIN_LED_VERDE, LOW);  
}

void acendeAlertaLedVermelho() {
  //Serial.println("Acender LED vermelho");
  digitalWrite(PIN_LED_ALERT_VERMELHO, HIGH);  
}

void apagaAlertaLedVermelho() {
  //Serial.println("Apagando LED vermelho");  
  digitalWrite(PIN_LED_ALERT_VERMELHO, LOW);  
}

void acendeAlertaLedAmarelo() {
  //Serial.println("Acender LED amarelo");  
  digitalWrite(PIN_LED_ALERT_AMARELO, HIGH);  
}

void apagaAlertaLedAmarelo() {
  //Serial.println("Apagando LED amarelo");  
  digitalWrite(PIN_LED_ALERT_AMARELO, LOW);  
}

void acendeAlertaLedVerde() {
  //Serial.println("Acender LED verde");  
  digitalWrite(PIN_LED_ALERT_VERDE, HIGH);  
}

void apagaAlertaLedVerde() {
  //Serial.println("Apagando LED verde");  
  digitalWrite(PIN_LED_ALERT_VERDE, LOW);  
}



