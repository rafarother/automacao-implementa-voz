//Incluir bibliotecas próprias do BitVoicer
#include <BVSP.h>
#include <BVSMic.h>
#include <Servo.h>


// Definiçao dos pinos que serão usados pelo Arduino
#define statusLED 5
#define periodoat 6
#define lampada1 4
#define entaudio  0
#define relelamp 7
Servo servomotor;



//Define as constantes que serão enviadas como parâmetro para a função bvsp.begin
const unsigned long STATUS_REQUEST_INTERVAL = 2000;
const unsigned long STATUS_REQUEST_TIMEOUT = 1000;



//Inicializar uma nova instância global da classe BVSP
BVSP bvsp = BVSP();

//Inicializar uma instância global da classe BVSMIC
BVSMic bvsm = BVSMic();

//Cria um buffer que será usado para ler amostras gravadas do BVSMIC, com um buffer de audio do tamanho de 64
byte audioBuffer[64];

//Cria um buffer de string para recever strings do BitVoicerServer, sendo que o seu tamanho é 64
char stringBuffer[64];

void setup() 
{
  //Inicia a comunicação serial em 115200 bps
  Serial.begin(115200);
  
  // Define os modos de pinagem do arduino
  pinMode(statusLED, OUTPUT);
  pinMode(periodoat, OUTPUT);
  pinMode(lampada1, OUTPUT);
  pinMode(relelamp, OUTPUT);
  
  // Define a porta serial do arduino que será usada na comunicação, quanto tempo levará até que uma solicitação de status expire
  //e com que frequência as solicitações de status devem ser enviadas ao BitVoicer Server
 
  bvsp.begin(Serial, STATUS_REQUEST_TIMEOUT, 
    STATUS_REQUEST_INTERVAL);
    

  // Define a função frameReceived
  bvsp.frameReceived = BVSP_frameReceived;
  
  // Prepara o time da classe do BVSMic
  bvsm.begin();

  // Configuração do servo motor para o porta 9
   servomotor.attach(9);   
}

void loop() 
{
  
  // Se o intervalo de solicitação do status expirar, envia nova solicitação de status pro BitVoicer Server
  bvsp.keepAlive();
  
  // Verifica se há dados disponíveis na porta serial
  
  bvsp.receive();
  

  
  // Obtém os status do servidor, verifica se há SRE disponível e verifica se há encaminhamento de dados
  if (bvsp.isSREAvailable()&& bvsp.isBVSRunning()&& bvsp.isDataFwdRunning() )
  {
    // Liga o LED de status
    digitalWrite(statusLED, HIGH);
    
    
    //Se o microfone não estiver ativo, ativa e começa a gravar
    if (!bvsm.isRecording)
    {
      bvsm.setAudioInput(entaudio, EXTERNAL);
      bvsm.startRecording();
    }
    
    // Verfica se tem amostras disponíveis 
    if (bvsm.available)
    {
      // Garante que o modo de entrada seja STREAM_MODE
      
      if (bvsp.inboundMode == FRAMED_MODE)
        bvsp.setInboundMode(STREAM_MODE);
      
      // Lê as amostras de audio do microfone
      int bytesRead = bvsm.read(audioBuffer, 64);
      
      // Envia o audio para o BitVoicer Server
      bvsp.sendStream(audioBuffer, bytesRead);
    }
  }
  else
  {

    // Se não tem SRE disponível, desliga o LED de status
    digitalWrite(periodoat, LOW);
    
    // Se o microfone está gravando, para
    if (bvsm.isRecording)
      bvsm.stopRecording();
  }
}


void BVSP_frameReceived(byte dataType, int payloadSize)
{

  //Executa a ação apropriada dependendo do tipo de dado enviado pelo servidor
  switch (dataType)
  {
    case DATA_TYPE_BYTE:
   
      // Liga ou desliga o LED de periodo ativo dependendo do valor recebido
      digitalWrite(periodoat, bvsp.getReceivedByte());
      break;
    case DATA_TYPE_STRING:
      // Executa ações adicionais somente se os caracteres forem reconhecidos 
      
      if (bvsp.getReceivedString(stringBuffer, 64) != 0);
      {
        // Caso seja recebido o string "pisca", as ações necessárias para os LEDs piscarão ativadas
        if (strcmp(stringBuffer, "blink") == 0)
        {
          desligaLED();
          delay(100);
          ligaLED();
          delay(100);
          desligaLED();
          delay(100);
          ligaLED();
          delay(100);
          desligaLED();
          delay(100);
          ligaLED();
        }

        //Cria comandos para acender e apagar a luz, quando receber o string igual ancede e apaga, respectivamente. 

        if (strcmp(stringBuffer, "acende") == 0)
        { digitalWrite(lampada1, HIGH);
          
         }
          if (strcmp(stringBuffer, "apaga") == 0)
        { digitalWrite(lampada1, LOW);
          
         }

         //Controle do servo motor.
          if (strcmp(stringBuffer, "fecha") == 0)
          {
            servomotor.write(20); //Move o servo pra posição 20 graus
            delay(15);
            }
         if (strcmp(stringBuffer, "pouco") == 0)
          {
            servomotor.write(80); //Move o servo para posição 80 graus
            delay(15);
            }
            if (strcmp(stringBuffer, "abre") == 0)
          {
            servomotor.write(150); //move o servo para a posição 150 graus
            delay(15);
            }
        //controle da lâmpada
        
         if (strcmp(stringBuffer, "acendel") == 0)
        { digitalWrite(relelamp, HIGH);
          
         }
          if (strcmp(stringBuffer, "apagal") == 0)
        { digitalWrite(relelamp, LOW);
          
         }
        
      }
      
      break;  
  }
}

// Liga todos os LEDs de status
void ligaLED()
{
  digitalWrite(statusLED, HIGH);
  digitalWrite(periodoat, HIGH);
 
}

// Desliga todos os LEDs de status
void desligaLED()
{
  digitalWrite(statusLED, LOW);
  digitalWrite(periodoat, LOW);
  
}
