//Testando git

/*
 * POSSIVEIS MELHORIAS:
 * USAR OUTRO ARDUINO PARA ACELERAR O CALCULO E ASSIM SENDO O PROCESSAMENTO DAS LEITURAS
 * PARA ISSO PRECISA USAR UMA COMUNICAÇÃO SERIAL
 * 
 * VARIAVEL PARA ESCOLHER NUMERO DE CASAS DECIMAIS
 */

//================================================
//               BIBLIOTECAS

//NEXTION
#include <doxygen.h>
#include <NexButton.h>
#include <NexConfig.h>
#include <NexCrop.h>
#include <NexGauge.h>
#include <NexHardware.h>
#include <NexHotspot.h>
#include <NexObject.h>
#include <NexPage.h>
#include <NexPicture.h>
#include <NexProgressBar.h>
#include <NexSlider.h>
#include <NexText.h>
#include <Nextion.h>
#include <NexTouch.h>
#include <NexWaveform.h>

//RTC - São necessárias as duas
#include <Wire.h>
#include "RTClib.h"

//SD - São necessárias as duas
#include <SPI.h>
#include <SD.h>

//=================================================
//              VARIAVEIS GLOBAIS

//Pinos
#define PinBuzzer 12  //Pino que conecta o buzzer
#define PinSensor A0  //Pino que conecta o módulo pneumático do pai
#define chipSelect 53 //Pino do SD

//Variaveis costumisaveis
#define delayDinam 100 //Delay em millis do dinamometro para ler botões
#define durationZerarDinam 1000 //Quantos millis o dinamometro vai demorar para capturar a pressão ambiente para desconsiderar do calculo
#define durationErroPaciente 3000 //Quantos millis vai durar a mensagem de erro no menu analisador
#define delayPadrao 3000 //Quantos millis demora o delay padrão

String nomePaciente  = ""; //Variavel global que armazena o nome recolhido do paciente
int page;       //Variavel que controla em que página o display nextion está
RTC_DS1307 rtc; //Declara o objeto RTC
char buffer[50];//Declara o buffer
File myFile;    //inicia um objeto arquivo
String Arq = "";
String ArqParametros = "";
bool contCons[10] = {false,false,false,false,false,false,false,false,false,false}; //Variavel de controle para histórico
char mao;
String ForcaMax[4] = {"","","",""};
String TFmax[4] = {"","","",""};
String TRes[4] = {"","","",""};
String TempoTotal[4] = {"","","",""};
String ImPuLsO[4] = {"","","",""};
String ReSiStEnCia[4] = {"","","",""};
String TrAbAlHo[4] = {"","","",""};
String tAxImp[4] = {"","","",""};
String tAxRes[4] = {"","","",""};
String NameData[4] = {"","","",""};

//=================================================
//              VELOCIDADE
/*
 * ESQUEMINHA QUE NÃO SEI COMO, AUMENTA A VELOCIDADE DE LEITURA DA PORTA ANALÓGIA DO ARDUINO.
 * PAI QUE ACHOU NUM ARTIGO, TINHA MAIS DE UMA MANEIRA DE EXECUTAR, MAS PREFIRIMOS ESTA
 */
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

//=================================================
//        DECLARAÇÃO DE OBJETOS NEXTION
/*
 * EXEMPLO DE COMO FUNCIONA
 * tipObjeto Nome  = tipObjeto( página, id, "nome")
 * NexPage   page0 = NexPage  (0      ,  0, "page0");
 * 
 * NECESSÁRIO PARA TODOS OS OBJECTOS DO DISPLAU NEXTION QUE SE COMUNICARÃO COM O ARDUINO
 */

//_____________________________________________PAGINAS
NexPage page0    = NexPage(0, 0, "page0");
NexPage page1    = NexPage(1, 0, "page1");
NexPage page2    = NexPage(2, 0, "page2");
NexPage page3    = NexPage(3, 0, "page3");
NexPage page4    = NexPage(4, 0, "page4");
NexPage page5    = NexPage(5, 0, "page5");
NexPage page6    = NexPage(6, 0, "page6");
NexPage page7    = NexPage(7, 0, "page7");
NexPage page8    = NexPage(8, 0, "page8");
NexPage page9    = NexPage(9, 0, "page9");
NexPage page10   = NexPage(10, 0, "page10");
NexPage page11   = NexPage(11, 0, "page11");
NexPage page12   = NexPage(12, 0, "page12");
NexPage page13   = NexPage(13, 0, "page13");

//____________________________________________OBJ PAGE0
NexText CartaoSD                = NexText(0, 3, "t1");
NexButton menuInicial           = NexButton(0, 1, "b0");

//____________________________________________OBJ PAGE1
NexButton Dinamometro           = NexButton(1, 1, "b0");                                                                            
NexButton Analisador            = NexButton(1, 2, "b1");
NexText textoData               = NexText(1, 4, "DATE");

//____________________________________________OBJ PAGE2
NexButton back1                 = NexButton(2, 9, "b2");
NexButton zerarDinam            = NexButton(2, 8, "b1");
NexButton iniciarDinam          = NexButton(2, 7, "b0");
NexText forcaMaxDinam           = NexText(2, 3, "F");
NexText forcaDinam              = NexText(2, 4, "Fmax");
NexProgressBar barForcaMaxDinam = NexProgressBar(2, 1, "j0");
NexProgressBar barForcaDinam    = NexProgressBar(2, 2, "j1");

//____________________________________________OBJ PAGE3
NexButton back2                 = NexButton(3, 1, "b3");
NexButton Novo                  = NexButton(3, 3, "b2");
NexButton Cadastrado            = NexButton(3, 2, "b1");
NexText Nome                    = NexText(3, 30, "t0");
NexText texCadastro             = NexText(3, 31, "t1");
NexButton RESETAR               = NexButton(3, 32, "b27");

//____________________________________________OBJ PAGE4
NexButton sair                  = NexButton(4, 4, "b1");
NexButton Medicao               = NexButton(4, 2, "b1");
NexButton Historico             = NexButton(4, 3, "b2");
NexButton back3                 = NexButton(4, 1, "b0");

//____________________________________________OBJ PAGE5
NexButton back4                 = NexButton(5, 1, "b0");
NexButton Direita               = NexButton(5, 3, "b2");
NexButton Esquerda              = NexButton(5, 2, "b1");
NexText textMao                 = NexText(5, 4, "t0");

//____________________________________________OBJ PAGE6
NexButton back5                 = NexButton(6, 1, "b0");
NexButton bZErar                = NexButton(6, 2, "b1");
NexText pA                      = NexText(6, 5, "t1");
NexText AGUARDE                 = NexText(6, 6, "t2");

//____________________________________________OBJ PAGE7
NexWaveform grafico             = NexWaveform(7, 1, "s0");
NexButton back6                 = NexButton(7, 2, "b0");
NexButton refazer               = NexButton(7, 3, "b1");

//____________________________________________OBJ PAGE8
NexText fMax_nex                = NexText(8,9 , "t9");
NexText T1_nex                  = NexText(8,8 , "t8");
NexText T2_nex                  = NexText(8,10 , "t10");
NexText T3_nex                  = NexText(8,11 , "t11");
NexText IMPULSO_nex             = NexText(8, 12, "t12");
NexText RESISTENCIA_nex         = NexText(8, 14, "t14");
NexText TRABALHO_nex            = NexText(8, 13, "t13");
NexText taxaResistencia         = NexText(8, 16, "t7");
NexText taxaImpulso             = NexText(8, 19, "t17");
NexButton back7                 = NexButton(8, 15, "b0");

//____________________________________________OBJ PAGE9
NexButton back8                 = NexButton(9, 3, "b2");
NexButton esq                   = NexButton(9, 2, "b1");
NexButton dir                   = NexButton(9, 1, "b0");

//____________________________________________OBJ PAGE10
NexText consulta1               = NexText(10, 8, "t2");
NexText consulta2               = NexText(10, 9, "t3");
NexText consulta3               = NexText(10, 10, "t4");
NexText consulta4               = NexText(10, 6, "t0");
NexText consulta5               = NexText(10, 7, "t1");
NexText consulta6               = NexText(10, 3, "t7");
NexText consulta7               = NexText(10, 4, "t8");
NexText consulta8               = NexText(10, 5, "t9");
NexText consulta9               = NexText(10, 1, "t5");
NexText consulta10              = NexText(10, 2, "t6");
NexButton back9                 = NexButton(10, 11, "b0");
NexButton Reset                 = NexButton(10, 12, "b1");
NexButton Delete                = NexButton(10, 13, "b2");
NexButton graficoTabela         = NexButton(10, 14, "b3");
NexText texTabela               = NexText(10, 25, "t10");

//____________________________________________OBJ PAGE12
NexText fMax_1                = NexText(12,9 , "t9");
NexText T1_1                  = NexText(12,8 , "t8");
NexText T2_1                  = NexText(12,10 , "t10");
NexText T3_1                  = NexText(12,11 , "t11");
NexText IMPULSO_1             = NexText(12, 12, "t12");
NexText RESISTENCIA_1         = NexText(12, 14, "t14");
NexText TRABALHO_1            = NexText(12, 13, "t13");
NexText taxaResistencia_1     = NexText(12, 16, "t7");
NexText taxaImpulso_1         = NexText(12, 19, "t17");
NexText fMax_2                = NexText(12,21 , "t19");
NexText T1_2                  = NexText(12,20 , "t18");
NexText T2_2                  = NexText(12,22 , "t20");
NexText T3_2                  = NexText(12,23 , "t21");
NexText IMPULSO_2             = NexText(12, 24, "t22");
NexText RESISTENCIA_2         = NexText(12, 26, "t24");
NexText TRABALHO_2            = NexText(12, 25, "t23");
NexText taxaImpulso_2         = NexText(12, 28, "t26");
NexText taxaResistencia_2     = NexText(12, 27, "t25");
NexText fMax_3                = NexText(12,39 , "t37");
NexText T1_3                  = NexText(12,38 , "t36");
NexText T2_3                  = NexText(12,40 , "t38");
NexText T3_3                  = NexText(12,41 , "t39");
NexText IMPULSO_3             = NexText(12, 42, "t40");
NexText RESISTENCIA_3         = NexText(12, 44, "t42");
NexText TRABALHO_3            = NexText(12, 43, "t41");
NexText taxaImpulso_3         = NexText(12, 46, "t44");
NexText taxaResistencia_3     = NexText(12, 45, "t43");
NexText fMax_4                = NexText(12,30 , "t28");
NexText T1_4                  = NexText(12,29 , "t27");
NexText T2_4                  = NexText(12,31 , "t29");
NexText T3_4                  = NexText(12,32 , "t30");
NexText IMPULSO_4             = NexText(12, 33, "t31");
NexText RESISTENCIA_4         = NexText(12, 35, "t33");
NexText TRABALHO_4            = NexText(12, 34, "t32");
NexText taxaImpulso_4         = NexText(12, 37, "t35");
NexText taxaResistencia_4     = NexText(12, 36, "t34");
NexText NomeData1             = NexText(12, 47, "t45");
NexText NomeData2             = NexText(12, 48, "t46");
NexText NomeData3             = NexText(12, 49, "t47");
NexText NomeData4             = NexText(12, 50, "t48");
NexText Select_FMAX           = NexText(12, 2, "t1");
NexText Select_T1             = NexText(12, 1, "t0");
NexText Select_T2             = NexText(12, 3, "t2");
NexText Select_T3             = NexText(12, 4, "t3");
NexText Select_Imp            = NexText(12, 5, "t4");
NexText Select_Res            = NexText(12, 7, "t6");
NexText Select_Tra            = NexText(12, 6, "t5");
NexText Select_taxImp         = NexText(12, 17, "t15");
NexText Select_taxRes         = NexText(12, 18, "t16");
NexButton back10              = NexButton(12, 15, "b0");

//____________________________________________OBJ PAGE13
NexWaveform graficoParametro  = NexWaveform(13, 1, "s0");
NexButton back11             = NexButton(13, 2, "b0");
NexButton MenuParam           = NexButton(13, 3, "b2");
NexText TipoGrafico           = NexText(13, 4, "t0"); 
//====================================================
//            LOOPS TOUCH NEXTION
/*
 * TODOS OS OBJETOS NEXTION QUE AO SEREM TOCADOS MANDARÃO ALGUMA INFORMAÇÃO OU SINAL AO ARDUINO
 * NECESSITAM DE SER DECLARADOS NESTES LOOPS.
 * FORAM FEITOS UM PARA CADA PÁGINA, POR QUE SE TIVER MUITOS OBJETOS DECLARADOS NUM LOOP SÓ PODE SER QUE AO TOCAR O 
 * O OBJETO O ARDUINO NÃO LEIA O TOQUE, ISSO OCORRE PORQUE O ARDUINO FICA VERIFICANDO TODOS OS COMPONENTES DA TELA.
 */

NexTouch *Page0[] =
{
      &menuInicial,
      NULL
};

NexTouch *Page1[] =
{
      &Analisador,
      &Dinamometro,
      NULL
};

NexTouch *Page2[] =
{
      &back1,
      &zerarDinam,
      &iniciarDinam,
      NULL
};

NexTouch *Page3[] =
{
      &back2,
      &Novo,
      &Cadastrado,
      &RESETAR,  
      NULL
};

NexTouch *Page4[] =
{
      &sair,
      &Medicao,
      &Historico,
      &back3,
      NULL
};

NexTouch *Page5[] =
{
      &back4,
      &Direita,
      &Esquerda,
      NULL
};

NexTouch *Page6[] =
{
      &back5,
      &bZErar,
      NULL
};

NexTouch *Page7[] =
{
      &back6,
      &refazer,
      NULL
};

NexTouch *Page8[] =
{
      &back7,
      NULL
};

NexTouch *Page9[] =
{
      &back8,
      &esq,
      &dir,
      NULL
};

NexTouch *Page10[] =
{
      &consulta1,
      &consulta2,
      &consulta3,
      &consulta4,
      &consulta5,
      &consulta6,
      &consulta7,
      &consulta8,
      &consulta9,
      &consulta10,
      &back9,
      &Reset,
      &Delete,
      &graficoTabela,
      NULL
};

NexTouch *Page11[] =
{
      NULL
};

NexTouch *Page12[] =
{     &Select_FMAX,        
      &Select_T1,
      &Select_T2,            
      &Select_T3,            
      &Select_Imp,           
      &Select_Res,          
      &Select_Tra,           
      &Select_taxImp,         
      &Select_taxRes,                 
      &back10,
      NULL
};

NexTouch *Page13[] =
{
      &back11,
      &MenuParam,   
      NULL
};

//=================================================
//                  FUNÇÕES

String returnData() { 
//FUNÇÃO QUE RETORNA A DATA, BUSCANDO A INFORMAÇÃO NO RTC
//OBS CASO PRECISE AJUSTAR A DATA E HORA DO RTC USE
//rtc.adjust(DateTime(2019, 9, 17, 10, 40, 0));
  
  String ERRO = "ERRO RTC";               //CRIA MENSAGEM DE ERRO
  String DATA = "";

  Serial.end();         //ENCERRA A COMUNICAÇÃO ANTERIOR
  Serial.begin(57600); //POIS PARA SE COMUNICAR COM O RTC TEM QUE MUDAR O BAUDRATE
   
  if (! rtc.begin()) { //VERIFICA SE TA TUDO CERTO COM RTC
    while (1);
    textoData.setText("ERRO RTC");              //ENVIA MENSAGEM DE ERRO PARA OBJ TEXTO DO NEXTION
  }
  if (! rtc.isrunning()) {  //VERIFICA SE TA TUDO CERTO COM RTC
    textoData.setText("ERRO RTC");              //ENVIA MENSAGEM DE ERRO PARA OBJ TEXTO DO NEXTION
  }

  DateTime now = rtc.now();//NÃO LEMBRO PARA QUE SERVE, MAS DEVE INICIALIZAR O OBJETO RTC
  
  int ano = now.year();    //ARMAZENA ANO
  int mes = now.month();   //ARMAZENA MÊS
  int dia = now.day();     //ARMAZENA DIA

  Serial.end();         //ENCERRA A COMUNICAÇÃO ANTERIOR
  Serial.begin(115200); //POIS PARA SE COMUNICAR COM O RTC TEM QUE MUDAR O BAUDRATE
  
  //PERFUMARIA PARA DEIXAR A DATA PADRONIZADA
  //Obs: Vai concatenando os valores, ou seja vai adicionando os chars a direita da string
  if(dia < 10) {
    DATA += '0'+String(dia);
  } else {
    DATA += String(dia);
  }
  DATA += '/';
  if(mes < 10) {
    DATA += '0'+String(mes);
  } else {
    DATA += String(mes);
  }
  DATA += '/' + String(ano);

  return DATA; 
}

int zeragemDinam() {  
  //Função que reseta tudo do dinamometro
  digitalWrite(PinBuzzer, HIGH);           //liga buzzer
  unsigned long controleTime = millis();   //variavel de controle do tempo
  int Pressao;                             //variavel que ajusta o sensor a pressao
  barForcaDinam.setValue(0);               //ZERA TODOS OS OBJETOS                       
  barForcaMaxDinam.setValue(0);
  forcaDinam.setText("0");         
  forcaMaxDinam.setText("0");      
  while(millis() - controleTime <= durationZerarDinam) {
    Pressao = analogRead(PinSensor);       //ajusta o valor da pressao
    forcaDinam.setText("ZERANDO");         
    forcaMaxDinam.setText("ZERANDO");
   }
  digitalWrite(PinBuzzer, LOW);            //desliga buzzer
  return Pressao;                          //retorna a pressao
}

void showTextDinam(int leitura, int maximo) {
  //função que pega os dados trata e mostra no display
  nexLoop(Page2);
  float vMaxDin_conv =(0.0075*sq(maximo/10)+0.3404*(maximo/10)-0.2687); //armazena o valor maximo em kgf
  float leitura_conv =(0.0075*sq(leitura/10)+0.3404*(leitura/10)-0.2687);  //ar,azema a leitura atual em kgf
  //observe que devido ao calculo executado para transformar em kgf, se inputar um 0 a formula retorna um numero negativo, por isso devemos zerar as transformações
  if(vMaxDin_conv < 0) {
    vMaxDin_conv = 0; //caso seja menor que zero será zero
  }
  if(leitura_conv < 0) {
    leitura_conv = 0; //caso seja menor que zero será zero
  }
  memset(buffer, 0, sizeof(buffer));  //limpa buffer
  //itoa(vMaxDin, buffer, 10);
  //dtostrf(var float, lenghth number, casas decimais, buffer)
  dtostrf(vMaxDin_conv, 5, 1, buffer);//converte float para array de chars e armazena no buffer      
  forcaMaxDinam.setText(buffer);      //envia valor do buffer para objeto text mostrar 
  memset(buffer, 0, sizeof(buffer));  //limpa buffer
  dtostrf(leitura_conv, 5, 1, buffer);//converte float para array de chars e armazena no buffer      
  forcaDinam.setText(buffer);         //envia valor do buffer para objeto text mostrar 
}

void showBarDinam(int leitura, int maximo, int pressao) {
  //Função que manda as informações para o objeto progress bar do arduino
  nexLoop(Page2);
  int controleTamProgressBar = 1023 - pressao;                       //monta a escala da progress bar
  int vMax_graf = map(maximo, 0, controleTamProgressBar, 0, 100);    //cria uma variavel propocional a progress bar
  int leitura_graf = map(leitura, 0, controleTamProgressBar, 0, 100);
  barForcaDinam.setValue(leitura_graf);                              //Manda para a progress bar
  barForcaMaxDinam.setValue(vMax_graf);
}

void leituraDinam(int Pressao) {                  
  //função que le a força do sensor
  int leituraDinam;                               //armazena 0-1023 a leitura da entrada analogica
  int vMaxDin = 0;                                //armazena 0-1023 a maior leitura da entrada analogica, zerando do a primeiramente
  while(page == 2) {                       
    nexLoop(Page2);                               //Sem esse comando, enquanto dentro do while loop o nextion não seria capaz de ler os touch events
    leituraDinam = (analogRead(PinSensor)-Pressao);
    vMaxDin = max(vMaxDin, leituraDinam);         //Armazena o valor maximo, entre o valor maximo atual e a leitura atual
    showTextDinam(vMaxDin, leituraDinam);         //Função que escreve a leitura e valor maximo no display
    showBarDinam(vMaxDin, leituraDinam, Pressao); //Função que escreve a leitura e valor maximo na progress bar
    delay(delayDinam);  //Delay para conseguir ler os botões
  }
  
}

void showData() {                     
  //Mostra a data no display
  memset(buffer, 0, sizeof(buffer));  //Limpa o buffer
  String Data = returnData();         //Armazena a string da função
  Data.toCharArray(buffer,sizeof(buffer));//Envia a string pro buffer
  textoData.setText(buffer);          //Manda a string do buffer para o objeto mostrar
}

String getNomePaciente() {
  //Pega o nome do paciente da caixa de texto
  //Detalhe importante o nome é escrito por pura programação no nextion, o arduino só retira
  memset(buffer, 0, sizeof(buffer));                        //Limpa buffer
  Nome.getText(buffer, sizeof(buffer));                     //Pega o nome do paciente
  String buf = '/' + String(buffer);                        //Adiciona barra
  return buf;
}

void (*resetFunction)() = 0;  //Função que reseta o arduino

void selectMao(char Mao) {
  //Função que mostra a mão que foi selecionada
  String DATE = returnData();   //pega a data
  DATE.remove(2,1);             //remove as barras
  DATE.remove(4,1);
  Arq = nomePaciente + '/' + DATE + '/' + String(Mao) + ".txt"; //cria a string com diretorio
  ArqParametros = nomePaciente + '/' + DATE + "/Param" + String(Mao) + ".txt"; //cria a string com diretorio
  if(SD.exists(Arq)) {   //verifica se já existe a medicao
    textMao.setText("Ja existe uma leitura dessa mao");
    digitalWrite(PinBuzzer, HIGH);
    delay(delayPadrao);
    digitalWrite(PinBuzzer, LOW);
    textMao.setText("");
  } else {
    page6.show();
    page = 6;
  }
}

void verGrafico(int nLeituras, int pressao) {
  page7.show(); //Mostra pagina 7
  page = 7;     //Variavel de controle da pagina
  int controleTamWave = 1023 - pressao;  //Controla a escala gráfica que sera usada na waveform
  int Proporcao = nLeituras/620;         //Divide a quantidade de leituras feitas, pelo espaço na waveform 
  int y = 0;                             //controle de qual leitura está
  String linhaGraf;
  unsigned long media = 0;
  unsigned long controleMedia = 0;
  if(SD.exists(Arq)){  //Confere se o arquivo existe
    Serial.println(Arq + " Existe");
  }
  myFile = SD.open(Arq, FILE_READ); //Abre o arquivo para leitura
  while (myFile.available()>0) {  //Enquanto tem oq ler no arquivo executa while
    char T = myFile.read();       //Armazena o caracter que está sendo lido
                                  //IMPORTANTE: O programa le caracter a caracter
    if(isPunct(T) || isDigit(T)) {//Se é número ou ponto ele concatena numa string
      linhaGraf.concat(T);  //concatenando na string
    } else{ //Se não é ponto ou digito, é quebra de linha "\n", nesse caso a gente retira a leitura      
      if(linhaGraf != "") { //Só para garantir que não esta vazio a string
        y+=1; //Contabiliza a leitura
        //Lembre se que está gravado tempo|leitura, tempo em micros, leitura em byte (0 - 1023)
        int espacoString1 = linhaGraf.indexOf('|'); //Procura em qual espaço esta a divisória
        String stringLeit = linhaGraf.substring(espacoString1+1); //Retira somente a leitura
        int leitura = stringLeit.toInt();  //Converte a leitura de string para int
        linhaGraf = ""; //Limpa a string para as próximas linhas
        media += leitura; //Soma na média as leituras
        if(controleMedia <= 620) { //Enquanto leu menos de 620 médias vem pra ca
          if(y%Proporcao == 0) {   //Se esta na proporção ele vai calcular a media e mandar pro gráfico
            controleMedia+=1;      //Contabiliza quantas médias foram calculadas
            media = media/Proporcao;//Calcula a média 
            int leituraGraf = map(media, 0, controleTamWave, 0, 255);//Converte a leitura em bytes para o espaço do gráfico
            grafico.addValue(0, leituraGraf); //Manda o resultado da média para o gráfico
            media = 0; //Zera a média
          }} else{ //Essa é uma tentativa de conseguir mandar a ultima leitura diretamente pro grafico, sem tirar média
            if(y == (nLeituras-1)) {
              int leituraGraf = map(leitura, 0, controleTamWave, 0, 255); //Converte pro espaço do gráfico
              grafico.addValue(0, leituraGraf); //Envia para o gráfico
            }
          }
      }}
  }
  myFile.close();  
}

void leituraAnalisador(int pressao) { //Função responsável pela leitura no Analisador
  myFile = SD.open(Arq, FILE_WRITE);  //Abre o arqvuivo
  if(SD.exists(Arq)) { //Se o arquivo existe roda função de leitura
   pA.setText(""); //Limpa texto
   int pressao_Leit = pressao; //armazena a pressão que foi lida anteriormente
   bool ler = true;    //variavel de controle para loop while
   unsigned long x = 0; //conta quantas leituras
   unsigned int leitura = 0;
   unsigned int vMax = 0;
   AGUARDE.setText("PRESSIONE"); //Manda pressinar
   digitalWrite(PinBuzzer, HIGH); //Toca buzzer
   delay(50); //Delayzinho padrão
   unsigned long tempoFim;
   unsigned long temporizador = millis(); //Variavel que vai controlar o tempo, para no minimo o programa parar de ler
                                           //após 2000 millis de leitura
   unsigned long tempoInicio = micros();  //variavel de controle para saber quanto tempo demorou cada leitura
   while(ler == true) { //Enquanto ler for true ele le
    x++; //contagem de leituras
    vMax = max(vMax, leitura); //armazena o valor maximo lido
    int tempoLeitura = micros() - tempoInicio; //armazena o tempo pra cada leitura
    tempoInicio = micros(); //zera o tempo
    leitura = analogRead(PinSensor)-pressao_Leit; //leitura - pressão anterior
    String linha = String(tempoLeitura) + '|' + String(leitura); //junta tudo numa mesma linha
    //Serial.println(linha);
    myFile.println(linha); //Grava no sd, "TEMPO|LEITURA"
    if(leitura <= (vMax/2) && (millis() - temporizador) >= 2000) {
      //Caso a leitura seja menor ou igual a metade da maior leitura e
      //tiver se passado no minimo 2000 millis, o programa encerra a leitura
      myFile.close(); //fecha arquivo
      myFile = SD.open(ArqParametros, FILE_WRITE); //Abre o arquivo onde armazenaremos os parametros
      myFile.println(vMax); //Salva maior leitura
      myFile.println(x);    //Salva quantas leituras ocorreram
      myFile.close();       //Fecha o arquivo
      //Serial.println("Força Máxima: " + String(vMax));
      //Serial.println("Força Máxima/2: " + String(vMax/2));
      //Serial.println("Ult Leitura: " + String(leitura));
      ler = false; //Encerra o loop
     }
    }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
    digitalWrite(PinBuzzer, LOW); //Desliga o buzzer
    verGrafico(x, pressao_Leit);      
  } else {
    AGUARDE.setText("Problema com arquivo"); //Se não mostra mensagem de erro
  }
 }

 void regressaoLinear() {
  //OBSERVAÇÃO: TEMOS QUE CALIBRAR O APARELHO PARA VER SE ELE ESTA CALCULANDO E DANDO OS PARAMETROS CERTOS
  //INCLUSIVE NO TEMPO
  int y = 0;
  float forcaMaxima = 0;
  int NLeituras = 0;
  String linha = "";
  if(SD.exists(ArqParametros)){  //Confere se o arquivo com os parametros existe
    Serial.println(ArqParametros + " Existe");
  }
  myFile = SD.open(ArqParametros, FILE_READ); //Abre o arquivo para leitura, a gente vai retirar força maxima e quantidade de leituras
  while (myFile.available()>0) {  //Enquanto tem oq ler no arquivo executa while
   char T = myFile.read();       //Armazena o caracter que está sendo lido
                                 //IMPORTANTE: O programa le caracter a caracter
   if(isDigit(T)) {//Se é número ele concatena numa string
    linha.concat(T);  //concatenando na string
   } else{ //Se não é digito, é quebra de linha "\n", nesse caso a gente retira os parametros      
     if(linha != "") { //Só para garantir que não esta vazio a string
      if(y == 0) { //Primeira linha é a força maxima
        forcaMaxima = linha.toInt(); //converte de string pra int e armazena
        Serial.println("forcaMaxima: " + String(forcaMaxima));
        y+=1;
      } else { //segunda linha é o número de leituras
         NLeituras = linha.toFloat(); //converte de string pra float e armazena
         Serial.println("nLeituras: " + String(NLeituras));
        }
        linha = "";
     }
    }
  }
  myFile.close(); //Fecha o arquivo com os parametros
  myFile = SD.open(Arq);  //Abre o arquivo com as leituras
  bool IMPULSO = true;
  bool oneTime = true;
  float tempo = 0;
  float leitura = 0;
  float leituraConv = 0;
  float T1 = 0;
  float T2 = 0;
  float T3 = 0;
  float Impulso = 0;
  float Resistencia = 0;
  float Trabalho = 0;
  int NLeiturasI = 0;
  int NLeiturasR = 0;
  linha = "";
  y = 0;
  while (myFile.available()>0) { //Le o arquivo
    char T = myFile.read();  //le os caracteres um por um
    if(isPunct(T) || isDigit(T)) { //Se for ponto ou digito armazena na linha
    linha.concat(T); //linha TEMPO|LEITURA (Leitura em bytes)
    } 
      if(isControl(T)) { //Se é quebra de linha
        if(linha != "") { //Garante que não é uma linha vazia
          int espacoString1 = linha.indexOf('|'); //Armazena o espaço onde esta a divisória
          String stringTime = linha.substring(0, espacoString1); //Pega o tempo e armazena numa string
          String stringLeit = linha.substring(espacoString1+1);  //Pega a leitura e armazena numa string, OBS: Aqui é "espacoString1+1" para não ler a divisória
          linha = ""; //Limpra a linha
          tempo = stringTime.toFloat();  //Converte a string tempo em float
          tempo = tempo/1.988; //Não lembro pq fazemos isso
          tempo = tempo/1000;
          leitura = stringLeit.toFloat(); 
          leituraConv = (0.0075*sq(leitura/10)+0.3404*(leitura/10)-0.2687); //converte a leitura em kgf
          y++; //Vai contando quantas leituras foram
        }
        if(leitura == forcaMaxima && oneTime == true) {
          //Se atingiu a força maxima ele para de contabilizar o impulso
          //e começa a contabilizar a resistencia
          //tem uma variavel booleana para garantir que ele só
          //ira executar isto uma vez
          oneTime = false; //Foi mais de uma vez
          IMPULSO = false; //Não é mais impulso
          NLeiturasI = y;  //Número de leituras para impulso é igual a y
          NLeiturasR = NLeituras - y; //O resto são leituras da resistência
        }
        if(IMPULSO == true) { //Se é impulso
          T1+=tempo; //Soma no tempo do impulso
          Impulso += leituraConv*(tempo/1000); //Soma as leituras no impulso
        } else {  //Se não é porque é resistência
          T2+=tempo; //Soma no tempo da resistência
          Resistencia += leituraConv*(tempo/1000); //Soma as leituras na resistencia
        }
      }
    }
  myFile.close(); //fecha o arquivo das leituras
  T3 = T1 + T2;  //Soma o tempo impulso mais tempo resistencia
  Trabalho = Impulso+Resistencia; //Soma as leituras de impulso e resistencia    
  forcaMaxima =(0.0075*sq(forcaMaxima/10)+0.3404*(forcaMaxima/10)-0.2687); //Converte a força maxima que estava em kgf
  float taxImp = forcaMaxima/(T1/1000.000);        //Calcula taxa de impulso
  float taxRes = (forcaMaxima/2.00)/(T2/1000.000); //Calcula taxa de resistencia
  T2 = T2/1000;//Conversao ms para millis;
  T3 = T3/1000;//Conversao ms para millis;

  myFile = SD.open(ArqParametros, FILE_WRITE); //Abre o arquivo com os parametros para guardar os novos parametros
  myFile.println(T1,1); //Grava os parametros com uma casa após a virgula - TODOS
  myFile.println(T2,1);
  myFile.println(T3,1);
  myFile.println(Impulso,1);
  myFile.println(Resistencia,1);
  myFile.println(Trabalho,1);
  myFile.println(taxImp,1);
  myFile.println(taxRes,1);
  myFile.close(); //Fecha o arquivo com os parametros

  //Vou escrever somente uma vez, pq é repetido
  memset(buffer, 0, sizeof(buffer)); //Zera o buffer
  dtostrf(T1, 7, 2, buffer);         //Converte o float em array de chars e manda pro buffer
                                     //O 7 é pq o array vai ter 6 espaços (1234.56 "Contando com o ponto")
                                     //o 2 é pq são duas casas após a vírgula
  T1_nex.setText(buffer); 
  memset(buffer, 0, sizeof(buffer));  
  dtostrf(T2, 7, 2, buffer);
  T2_nex.setText(buffer); 
  memset(buffer, 0, sizeof(buffer));  
  dtostrf(T3, 7, 2, buffer);
  T3_nex.setText(buffer); 
  memset(buffer, 0, sizeof(buffer));  
  dtostrf(forcaMaxima, 7, 2, buffer);        
  fMax_nex.setText(buffer); 
  memset(buffer, 0, sizeof(buffer));  
  dtostrf(Impulso, 7, 2, buffer);        
  IMPULSO_nex.setText(buffer); 
  memset(buffer, 0, sizeof(buffer));  
  dtostrf(Resistencia, 7, 2, buffer);        
  RESISTENCIA_nex.setText(buffer); 
  memset(buffer, 0, sizeof(buffer));  
  dtostrf(Trabalho, 7, 2, buffer);        
  TRABALHO_nex.setText(buffer); 
  memset(buffer, 0, sizeof(buffer));  
  dtostrf(taxImp, 7, 2, buffer);
  taxaImpulso.setText(buffer); 
  memset(buffer, 0, sizeof(buffer)); 
  dtostrf(taxRes, 7, 2, buffer);
  taxaResistencia.setText(buffer); 
 }

String retornaData (String data, char Mao) {
  //Função que retorna a data no formato DIA/MES/ANO
  String teste = nomePaciente + '/' + data + '/' + Mao + ".txt"; //Guarda o local do arquivo na variavel
  String string;
  if(SD.exists(teste)) { //Se o arquivo existe
    //Ex Data estava 25022001
    //O comando substring retira uma parte de uma string
    //Vc da a primeira posição(includente) e da a ultima posição(excludente)
    //25022001 - Data
    //01234567 - Posição na string
    string = data.substring(0, 2) + '/' + data.substring(2, 4) + '/' + data.substring(4);
    //Data se torna 25/02/2001
    }
    else { //Se não conseguir achar o arquivo das leituras na pasta ele manda erro
      string = "ERRO";  
    }
    return string; //Retorna o valor da string
}

 void printDirectory(File dir, char Mao) {
  int x = 1; //Variavel que conta no número de diretórios data
  String Data; //Variavel que armazena a data
  while (true) {
    File entry =  dir.openNextFile(); //Não sei direito como funciona
    if (! entry) {
      //Não sei pra q serve esta parte
      break;
    }     
    if (entry.isDirectory()) { //Se é um diretório
      switch(x%10) { //Só tem 10 locais para armazenar as datas, então aqui uso módulo, para
                     //pegar as 10 mais recentes
        case 1:
        Data = retornaData(String(entry.name()), Mao); //Pega a data neste formato DIA/MES/ANO
        memset(buffer, 0, sizeof(buffer));  //Limpa o buffer
        Data.toCharArray(buffer,sizeof(buffer)); //Armazena a data no buffer como array de char
        if(Data != "ERRO") { //Se não houve erro ao abrir o arquivo
          consulta1.setText(buffer);  //Manda para o nextion
          x++; //E contabiliza
        }
        //OBSERVAÇÃO, TUDO SE REPETE, O QUE MUDA É O OBJETO NEXTION
        //consulta1, consulta2, etç
        break;
        case 2:
        Data = retornaData(String(entry.name()), Mao);
        memset(buffer, 0, sizeof(buffer));  
        Data.toCharArray(buffer,sizeof(buffer));
        if(Data != "ERRO") {
          consulta2.setText(buffer);
          x++;
        }
        break;
        case 3:
        Data = retornaData(String(entry.name()), Mao);
        memset(buffer, 0, sizeof(buffer));  
        Data.toCharArray(buffer,sizeof(buffer));
        if(Data != "ERRO") {
          consulta3.setText(buffer);
          x++;
        }
        break;
        case 4:
        Data = retornaData(String(entry.name()), Mao);
        memset(buffer, 0, sizeof(buffer));  
        Data.toCharArray(buffer,sizeof(buffer));
        if(Data != "ERRO") {
          consulta4.setText(buffer);
          x++;
        }
        break;
        case 5:
        Data = retornaData(String(entry.name()), Mao);
        memset(buffer, 0, sizeof(buffer));  
        Data.toCharArray(buffer,sizeof(buffer));
        if(Data != "ERRO") {
          consulta5.setText(buffer);
          x++;
        }
        break;
        case 6:
        Data = retornaData(String(entry.name()), Mao);
        memset(buffer, 0, sizeof(buffer));  
        Data.toCharArray(buffer,sizeof(buffer));
        if(Data != "ERRO") {
          consulta6.setText(buffer);
          x++;
        }
        break;
        case 7:
        Data = retornaData(String(entry.name()), Mao);
        memset(buffer, 0, sizeof(buffer));  
        Data.toCharArray(buffer,sizeof(buffer));
        if(Data != "ERRO") {
          consulta7.setText(buffer);
          x++;
        }
        break;
        case 8:
        Data = retornaData(String(entry.name()), Mao);
        memset(buffer, 0, sizeof(buffer));  
        Data.toCharArray(buffer,sizeof(buffer));
        if(Data != "ERRO") {
          consulta8.setText(buffer);
          x++;
        }
        break;
        case 9:
        Data = retornaData(String(entry.name()), Mao);
        memset(buffer, 0, sizeof(buffer));  
        Data.toCharArray(buffer,sizeof(buffer));
        if(Data != "ERRO") {
          consulta9.setText(buffer);
          x++;
        }
        break;
        case 0:
        Data = retornaData(String(entry.name()), Mao);
        memset(buffer, 0, sizeof(buffer));  
        Data.toCharArray(buffer,sizeof(buffer));
        if(Data != "ERRO") {
          consulta10.setText(buffer);
          x++;
        }
        break; 
        }
      } 
      entry.close(); //fecha o arquivo
  } 
}


void getParametros(String Data, int contador) {
  String arquivo = nomePaciente + '/' + Data + "/Fmax" + String(mao) + ".txt";
  String FMAX = "";
  String t1 = "";
  String t2 = "";
  String t3 = "";
  String ImPulso = "";
  String ReSistencia = "";
  String TrAbalho = "";
  String TaxImpu = "";
  String TaxResi = "";
  myFile = SD.open(arquivo, FILE_READ);
  int linha = 0; 
  while (myFile.available()>0) {
    char T = myFile.read();
    if(T == '\n') {
      linha+=1;
    } switch(linha) {
        case 0:
          if(isPunct(T) || isDigit(T)) {
            FMAX.concat(T);
          }
        break;
        case 1:
          if(isPunct(T) || isDigit(T)) {
            t1.concat(T);
          }
        break;
        case 2:
          if(isPunct(T) || isDigit(T)) {
            t2.concat(T);
          }
        break;
        case 3:
          if(isPunct(T) || isDigit(T)) {
            t3.concat(T);
          }
        break;
        case 4:
          if(isPunct(T) || isDigit(T)) {
            ImPulso.concat(T);
          }
        break;
        case 5:
          if(isPunct(T) || isDigit(T)) {
            ReSistencia.concat(T);
          }
        break;
        case 6:
          if(isPunct(T) || isDigit(T)) {
            TrAbalho.concat(T);
          }
        break;
        case 7:
          if(isPunct(T) || isDigit(T)) {
            TaxImpu.concat(T);
          }
        break;
        case 8:
          if(isPunct(T) || isDigit(T)) {
            TaxResi.concat(T);
          }
        break; }
  }
      myFile.close();
      float FmaxConvertido = FMAX.toFloat();
      FmaxConvertido =(0.0075*sq(FmaxConvertido/10)+0.3404*(FmaxConvertido/10)-0.2687);
      String pastaData = Data.substring(0,2) + '/' + Data.substring(2,4) + '/' + Data.substring(4);
      pastaData.trim();
      switch(contador) {
      case 0:
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      dtostrf(FmaxConvertido, 5, 1, buffer);
      fMax_1.setText(buffer); //envia valor para 
      ForcaMax[0] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      t1.toCharArray(buffer,sizeof(buffer));
      T1_1.setText(buffer); //envia valor para
      TFmax[0] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      t2.toCharArray(buffer,sizeof(buffer));
      T2_1.setText(buffer); //envia valor para
      TRes[0] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer 
      t3.toCharArray(buffer,sizeof(buffer));    
      T3_1.setText(buffer); //envia valor para
      TempoTotal[0] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      ImPulso.toCharArray(buffer,sizeof(buffer));  
      IMPULSO_1.setText(buffer); //envia valor para
      ImPuLsO[0] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      ReSistencia.toCharArray(buffer,sizeof(buffer));
      RESISTENCIA_1.setText(buffer); //envia valor para
      ReSiStEnCia[0] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TrAbalho.toCharArray(buffer,sizeof(buffer));
      TRABALHO_1.setText(buffer); //envia valor para
      TrAbAlHo[0] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TaxResi.toCharArray(buffer,sizeof(buffer));
      taxaResistencia_1.setText(buffer); //envia valor para
      tAxRes[0] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TaxImpu.toCharArray(buffer,sizeof(buffer));
      taxaImpulso_1.setText(buffer); //envia valor para 
      tAxImp[0] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  // Clear the buffer, so we can start using it
      pastaData.toCharArray(buffer,sizeof(buffer));
      NomeData1.setText(buffer);
      NameData[0] = String(buffer);
      break;
      case 1:
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      dtostrf(FmaxConvertido, 5, 1, buffer);
      fMax_2.setText(buffer); //envia valor para 
      ForcaMax[1] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      t1.toCharArray(buffer,sizeof(buffer));
      T1_2.setText(buffer); //envia valor para
      TFmax[1] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      t2.toCharArray(buffer,sizeof(buffer));
      T2_2.setText(buffer); //envia valor para
      TRes[1] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer 
      t3.toCharArray(buffer,sizeof(buffer));    
      T3_2.setText(buffer); //envia valor para
      TempoTotal[1] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      ImPulso.toCharArray(buffer,sizeof(buffer));  
      IMPULSO_2.setText(buffer); //envia valor para
      ImPuLsO[1] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      ReSistencia.toCharArray(buffer,sizeof(buffer));
      RESISTENCIA_2.setText(buffer); //envia valor para
      ReSiStEnCia[1] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TrAbalho.toCharArray(buffer,sizeof(buffer));
      TRABALHO_2.setText(buffer); //envia valor para
      TrAbAlHo[1] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TaxResi.toCharArray(buffer,sizeof(buffer));
      taxaResistencia_2.setText(buffer); //envia valor para
      tAxRes[1] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TaxImpu.toCharArray(buffer,sizeof(buffer));
      taxaImpulso_2.setText(buffer); //envia valor para 
      tAxImp[1] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  // Clear the buffer, so we can start using it
      pastaData.toCharArray(buffer,sizeof(buffer));
      NomeData2.setText(buffer);    
      NameData[1] = String(buffer);
      break;
      case 2:
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      dtostrf(FmaxConvertido, 5, 1, buffer);
      fMax_3.setText(buffer); //envia valor para 
      ForcaMax[2] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      t1.toCharArray(buffer,sizeof(buffer));
      T1_3.setText(buffer); //envia valor para
      TFmax[2] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      t2.toCharArray(buffer,sizeof(buffer));
      T2_3.setText(buffer); //envia valor para
      TRes[2] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer 
      t3.toCharArray(buffer,sizeof(buffer));    
      T3_3.setText(buffer); //envia valor para
      TempoTotal[2] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      ImPulso.toCharArray(buffer,sizeof(buffer));  
      IMPULSO_3.setText(buffer); //envia valor para
      ImPuLsO[2] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      ReSistencia.toCharArray(buffer,sizeof(buffer));
      RESISTENCIA_3.setText(buffer); //envia valor para
      ReSiStEnCia[2] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TrAbalho.toCharArray(buffer,sizeof(buffer));
      TRABALHO_3.setText(buffer); //envia valor para
      TrAbAlHo[2] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TaxResi.toCharArray(buffer,sizeof(buffer));
      taxaResistencia_3.setText(buffer); //envia valor para
      tAxRes[2] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TaxImpu.toCharArray(buffer,sizeof(buffer));
      taxaImpulso_3.setText(buffer); //envia valor para
      tAxImp[2] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  // Clear the buffer, so we can start using it
      pastaData.toCharArray(buffer,sizeof(buffer));
      NomeData3.setText(buffer);
      NameData[2] = String(buffer);
      break;
      case 3:
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      dtostrf(FmaxConvertido, 5, 1, buffer);
      fMax_4.setText(buffer); //envia valor para 
      ForcaMax[3] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      t1.toCharArray(buffer,sizeof(buffer));
      T1_4.setText(buffer); //envia valor para
      TFmax[3] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      t2.toCharArray(buffer,sizeof(buffer));
      T2_4.setText(buffer); //envia valor para
      TRes[3] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer 
      t3.toCharArray(buffer,sizeof(buffer));    
      T3_4.setText(buffer); //envia valor para
      TempoTotal[3] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      ImPulso.toCharArray(buffer,sizeof(buffer));  
      IMPULSO_4.setText(buffer); //envia valor para
      ImPuLsO[3] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      ReSistencia.toCharArray(buffer,sizeof(buffer));
      RESISTENCIA_4.setText(buffer); //envia valor para
      ReSiStEnCia[3] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TrAbalho.toCharArray(buffer,sizeof(buffer));
      TRABALHO_4.setText(buffer); //envia valor para
      TrAbAlHo[3] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TaxResi.toCharArray(buffer,sizeof(buffer));
      taxaResistencia_4.setText(buffer); //envia valor para
      tAxRes[3] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
      TaxImpu.toCharArray(buffer,sizeof(buffer));
      taxaImpulso_4.setText(buffer); //envia valor para
      tAxImp[3] = String(buffer);
      memset(buffer, 0, sizeof(buffer));  // Clear the buffer, so we can start using it
      pastaData.toCharArray(buffer,sizeof(buffer));
      NomeData4.setText(buffer);    
      NameData[3] = String(buffer);
      break;}
}

String getDataLeitura(String buf) {
  buf.remove(2, 1);
  buf.remove(4, 1);
  return buf;
}

void backParametros() {
  
}
//=====================================================
//            BOTÕES NEXTION
/*
 * CONFIGURANDO AS FUNÇÕES QUE SERÃO EXECUTADAS AO ACIONAR DOS BOTÕES
 */
void menuInicialPushCallback(void *ptr) {          
  page1.show();                       //Vai para pagina 1
  page = 1;
  showData();
}

void DinamometroPushCallback(void *ptr) {          
  page2.show();                       //Vai para pagina 2
  page = 2;
}

void iniciarDinamPushCallback(void *ptr) {
  int Pressao = zeragemDinam();
  leituraDinam(Pressao);
}

void zerarDinamPushCallback(void *ptr) {
  int Pressao = zeragemDinam();
  leituraDinam(Pressao);
}

void back1PushCallback(void *ptr) {
  page1.show();
  page = 1;
  showData();
}

void AnalisadorPushCallback(void *ptr) {          
  page3.show();                       //Vai para pagina 3
  page = 3;
}

void NovoPushCallback(void *ptr){
  //Botão para criar nova pasta de paciente
  nomePaciente = getNomePaciente();
  if(SD.exists(nomePaciente)) {                             //Verifica se o paciente já está cadastrado
    texCadastro.setText("Este paciente ja esta cadastrado");//Caso esteja mostra mensagem de erro
    digitalWrite(PinBuzzer, HIGH);                          //Liga buzzer
    delay(durationErroPaciente);
    digitalWrite(PinBuzzer, LOW);                           //Desliga buzzer
    texCadastro.setText("O nome do paciente deve conter no máximo 8 letras"); //Volta texto original
    Nome.setText("");                                       //Zera o resto
  } else{                                                   //Caso o paciente seja mesmo novo
      SD.mkdir(nomePaciente);                               //Cria pasta com nome do paciente
      if(SD.exists(nomePaciente)) {                               //Se conseguiu criar vai para próxima pagina
        page4.show();
        page = 4;                                           //Caso não tenha conseguido manda mensagem de erro
       } else {
         texCadastro.setText("Erro ao fazer cadastro" );
         digitalWrite(PinBuzzer, HIGH);
         delay(durationErroPaciente);
         digitalWrite(PinBuzzer, LOW);
         texCadastro.setText("O nome do paciente deve conter no máximo 8 letras");
  }}
}

void CadastradoPushCallback  (void *ptr){
  //Botão para pacientes já cadastrados
  nomePaciente = getNomePaciente();
  if(SD.exists(nomePaciente)) {                           //Caso exista o arquivo vai para proxima pagina
    page4.show();
    page = 4;
  } else {
    texCadastro.setText("Erro ao fazer login" );          //Caso não, exibe mensagem de erro
    digitalWrite(PinBuzzer, HIGH);
    delay(durationErroPaciente);
    digitalWrite(PinBuzzer, LOW);
    texCadastro.setText("O nome do paciente deve conter no máximo 8 letras");
  }
}

void RESETARPushCallback (void *ptr){
  resetFunction();
}

void back2PushCallback (void *ptr){
  page1.show();  //mostra pagina
  page = 1;     
  showData(); 
}

void sairPushCallback (void *ptr) {
  resetFunction();
}
  
void MedicaoPushCallback (void *ptr) {
  page5.show();
  page = 5;
}
  
void HistoricoPushCallback (void *ptr) {
  page9.show();
  page = 9; 
}
  
void back3PushCallback (void *ptr) {
  page3.show();
  page = 3;
}

void back4PushCallback (void *ptr) {
  page4.show();
  page = 4;   
}

void DireitaPushCallback (void *ptr) {
  char Mao = 'D'; //Mao direita
  selectMao(Mao);
}

void EsquerdaPushCallback (void *ptr) {
  char Mao = 'E'; //Mao esquerda
  selectMao(Mao);
}

void back5PushCallback( void *ptr) {
  page5.show();
  page = 5;  
}

void bZErarPushCallback(void *ptr) {
 String DATE = returnData();   //pega a data
 DATE.remove(2,1);             //remove as barras
 DATE.remove(4,1);
 SD.mkdir(String(nomePaciente + '/' + DATE)); //Cria a pasta
 int pressao_Leit; //Variavel que armazena a pressão
 unsigned long controlePressao = millis(); //Variavel que armazena o t0
  while(millis()-controlePressao <= 2000) {//Durante 2000 millis o programa le a pressão
   pressao_Leit = analogRead(PinSensor);  
   pA.setText("Calculando P.a.");          //Manda texto para nextion
  }
 memset(buffer, 0, sizeof(buffer));  //zera buffer
 itoa(pressao_Leit, buffer, 10);     //manda valor pro buffer
 pA.setText(buffer);                 //envia a pressao para o nextion
 AGUARDE.setText("");                //Rotina que
 delay(1000);
 AGUARDE.setText("AGUARDE");
 delay(1000);
 AGUARDE.setText("");
 delay(1000);
 leituraAnalisador(pressao_Leit);
}

void refazerPushCallback(void *ptr) {
  SD.remove(Arq); //Deleta o arquivo com as leituras
  SD.remove(ArqParametros); //Deleta o arquivo com os parametros
  String DATE = returnData();   //pega a data
  DATE.remove(2,1);             //remove as barras
  DATE.remove(4,1);
  SD.rmdir(nomePaciente + '/' + DATE); //Se não houver nenhuma leitura no diretório data, ele exclui o diretório
  page6.show();
  page = 6;
}

void back6PushCallback(void *ptr) {
  page8.show();
  page = 8;
  regressaoLinear();  //Faz a regressão linear, e calcula os parametros
}

void back7PushCallback(void *ptr) {
  page4.show();
  page = 4;
}

void back8PushCallback(void *ptr) {
  page4.show();
  page = 4;
}

void esqPushCallback( void *ptr) {
  mao = 'E'; //Determina qual é a mão
  page10.show();  
  page = 10;      
  File Paciente = SD.open(nomePaciente); //Abre o arquivo do Paciente
  printDirectory(Paciente, mao);  //Executa função print
  Paciente.close();  //Fecha p arquivo do paciente
}

void dirPushCallback( void *ptr) {
  mao = 'D'; //Determina qual é a mão
  page10.show();
  page = 10;
  File Paciente = SD.open(nomePaciente); //Abre o arquivo do Paciente
  printDirectory(Paciente, mao); //Executa função print
  Paciente.close();  //Fecha p arquivo do paciente 
}

void back9PushCallback(void *ptr) {
  page9.show();
  page = 9;
}

void consulta1PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer)); 
  consulta1.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") {
    contCons[0] = true;
}}
void consulta2PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer)); 
  consulta2.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") {
    contCons[1] = true;
}}
void consulta3PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer));
  consulta3.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") { 
    contCons[2] = true;
}}
void consulta4PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer)); 
  consulta4.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") {
    contCons[3] = true;
}}
void consulta5PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer)); 
  consulta5.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") {
    contCons[4] = true;
}}
void consulta6PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer)); 
  consulta6.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") {
    contCons[5] = true;
}}
void consulta7PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer));
  consulta7.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") { 
    contCons[6] = true;
}}
void consulta8PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer)); 
  consulta8.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") {
    contCons[7] = true;
}}
void consulta9PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer)); 
  consulta9.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") {
    contCons[8] = true;
}}
void consulta10PushCallback(void*ptr) {
  memset(buffer, 0, sizeof(buffer)); 
  consulta10.getText(buffer,sizeof(buffer));
  String controleExistencia = String(buffer);
  if(controleExistencia != "....................") {
    contCons[9] = true;
}}

void ResetPushCallback(void*ptr) {
  for(int i=0; i <=9; i++) {
    contCons[i] = false;
  }
}

void graficoTabelaPushCallback(void*ptr) {
  int x = 0;
  String Data[10] = {"","","","","","","","","",""};
  for(int i=0;i<=9;i++) {
    switch(i) {
      case 0:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta1.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
      case 1:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta2.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 2:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta3.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 3:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta4.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 4:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta5.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 5:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta6.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 6:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta7.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 7:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta8.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 8:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta9.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 9:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta10.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
    }
  }
  int z = 0;
  for(int i = 0; i <= 9; i++) { //Conta quantas datas foram escolhidas
    if(contCons[i] == true) {
      z++; 
    } 
  }
  if(z == 0) { //Se não escolheram nenhuma data
    memset(buffer, 0, sizeof(buffer));  
    texTabela.getText(buffer, sizeof(buffer));  
    texTabela.setText("Escolha pelo menos uma leitura");
    delay(3000);
    texTabela.setText(buffer);
  } else if(z <= 4){
    page12.show();
    page = 12;
    for(int i = 0; i < z; i++) {
      getParametros(Data[i], i);
    }
  } else {
    memset(buffer, 0, sizeof(buffer));  
    texTabela.getText(buffer, sizeof(buffer));  
    texTabela.setText("Escolha somente 4 leituras");
    delay(3000);
    texTabela.setText(buffer);
    int z = 0;
    page = 10;
    for(int i = 0; i < 4; i++) {
      Data[i] = "";
    }
  }
}

void back10PushCallback(void *ptr) {
  for(int i=0; i <=9; i++) { //Reseta todos
    contCons[i] = false;
  }
  for(int i = 0; i < 4; i++) {
    ForcaMax[i] = "";
    TFmax[i] = "";
    TRes[i] = "";
    TempoTotal[i] = "";
    ImPuLsO[i] = "";
    ReSiStEnCia[i] = "";
    TrAbAlHo[i] = "";
    tAxImp[i] = "";
    tAxRes[i] = "";
    NameData[i] = "";
  }
  page10.show();
  page = 10;
  File Paciente = SD.open(nomePaciente); //Abre o arquivo do Paciente
  printDirectory(Paciente, mao);  //Executa função print
  Paciente.close();  //Fecha p arquivo do paciente
}      

void DeletePushCallback(void *ptr) {
  int x = 0;
  String Data[10] = {"","","","","","","","","",""};
  for(int i=0;i<=9;i++) {
    switch(i) {
      case 0:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta1.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
      case 1:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta2.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 2:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta3.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 3:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta4.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 4:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta5.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 5:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta6.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 6:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta7.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 7:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta8.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 8:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta9.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
        case 9:
        if(contCons[i] == true) {
          memset(buffer, 0, sizeof(buffer)); 
          consulta10.getText(buffer,sizeof(buffer));
          Data[x] = getDataLeitura(String(buffer));
          x++;
        }
        break;
    }
  }
  for(int i = 0; i < x; i++) {
    String arquivo = nomePaciente + '/' + Data[i] + "/Fmax" + String(mao) + ".txt";
    SD.remove(arquivo);
    arquivo = nomePaciente + '/' + Data[i] + '/' + String(mao) + ".txt";
    SD.remove(arquivo);
    arquivo = nomePaciente + '/' + Data[i];
    SD.rmdir(arquivo);
  }
  consulta1.setText("....................");
  consulta2.setText("....................");
  consulta3.setText("....................");
  consulta4.setText("....................");
  consulta5.setText("....................");
  consulta6.setText("....................");
  consulta7.setText("....................");
  consulta8.setText("....................");
  consulta9.setText("....................");
  consulta10.setText("....................");
  File Paciente = SD.open(nomePaciente);
  printDirectory(Paciente, mao);
  Paciente.close();
  for(int i=0; i <=9; i++) {
    contCons[i] = false;
  }
  if(x == 0) {
    memset(buffer, 0, sizeof(buffer));  // Clear the buffer, so we can start using it
    texTabela.getText(buffer, sizeof(buffer));  // Read the text on the object t7 and store it on the buffer
    texTabela.setText("Escolha pelo menos uma leitura");
    delay(3000);
    texTabela.setText(buffer);
  }
}

void Select_FMAXPushCallback(void *ptr) {
  float leituraGraf[4] = {0,0,0,0};
  memset(buffer, 0, sizeof(buffer)); 
  fMax_1.getText(buffer,sizeof(buffer));
  String Valor = String(buffer);
  leituraGraf[0] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  fMax_2.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[1] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  fMax_3.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[2] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  fMax_4.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[3] = Valor.toFloat();
  float teto = 0;
  for(int i = 0; i < 4; i++) {
    teto = max(leituraGraf[i], teto);
  }
  teto + 5;
  page13.show();
  page = 13;
  TipoGrafico.setText("Força Max:");
  float proporcao;
  float Somatorio = 0;
  for(int i = 0; i < 4; i++) {
    if(leituraGraf[i] > 0) {
      if(i == 0) {
        proporcao = float(leituraGraf[0])/float(155);
      } else {
        proporcao = leituraGraf[i] - leituraGraf[i-1];
        Serial.println("Diferenca: " + String(proporcao));
        proporcao = float(proporcao)/float(155);
      }
      for(int y = 0; y <= 155; y++) {
        Somatorio+=proporcao;
        int posicao = map(Somatorio, 0, teto, 0, 255);
        graficoParametro.addValue(0, posicao);
      }
    }
  }
}

void Select_T1PushCallback(void *ptr) {
  long leituraGraf[4] = {0,0,0,0};
  memset(buffer, 0, sizeof(buffer)); 
  T1_1.getText(buffer,sizeof(buffer));
  String Valor = String(buffer);
  leituraGraf[0] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  T1_2.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[1] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  T1_3.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[2] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  T1_4.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[3] = Valor.toInt();
  long teto = 0;
  for(int i = 0; i < 4; i++) {
    teto = max(leituraGraf[i], teto);
  }
  teto + 5;
  page13.show();
  page = 13;
  TipoGrafico.setText("Tempo Fmax:");
  float proporcao;
  float Somatorio = 0;
  for(int i = 0; i < 4; i++) {
    if(leituraGraf[i] > 0) {
      if(i == 0) {
        proporcao = float(leituraGraf[0])/float(155);
      } else {
        proporcao = leituraGraf[i] - leituraGraf[i-1];
        Serial.println("Diferenca: " + String(proporcao));
        proporcao = float(proporcao)/float(155);
      }
      for(int y = 0; y <= 155; y++) {
        Somatorio+=proporcao;
        int posicao = map(Somatorio, 0, teto, 0, 255);
        graficoParametro.addValue(0, posicao);
      }
    }
  }
}
void Select_T2PushCallback(void *ptr) {
  float leituraGraf[4] = {0,0,0,0};
  memset(buffer, 0, sizeof(buffer)); 
  T2_1.getText(buffer,sizeof(buffer));
  String Valor = String(buffer);
  leituraGraf[0] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  T2_2.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[1] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  T2_3.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[2] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  T2_4.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[3] = Valor.toFloat();
  long teto = 0;
  for(int i = 0; i < 4; i++) {
    teto = max(leituraGraf[i], teto);
  }
  teto + 5;
  page13.show();
  page = 13;
  TipoGrafico.setText("Tempo Resist:");
  float proporcao;
  float Somatorio = 0;
  for(int i = 0; i < 4; i++) {
    if(leituraGraf[i] > 0) {
      if(i == 0) {
        proporcao = float(leituraGraf[0])/float(155);
      } else {
        proporcao = leituraGraf[i] - leituraGraf[i-1];
        Serial.println("Diferenca: " + String(proporcao));
        proporcao = float(proporcao)/float(155);
      }
      for(int y = 0; y <= 155; y++) {
        Somatorio+=proporcao;
        int posicao = map(Somatorio, 0, teto, 0, 255);
        graficoParametro.addValue(0, posicao);
      }
    }
  }
}
void Select_T3PushCallback(void *ptr) {
  float leituraGraf[4] = {0,0,0,0};
  memset(buffer, 0, sizeof(buffer)); 
  T3_1.getText(buffer,sizeof(buffer));
  String Valor = String(buffer);
  leituraGraf[0] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  T3_2.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[1] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  T3_3.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[2] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  T3_4.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[3] = Valor.toFloat();
  long teto = 0;
  for(int i = 0; i < 4; i++) {
    teto = max(leituraGraf[i], teto);
  }
  teto + 5;
  page13.show();
  page = 13;
  TipoGrafico.setText("Tempo Trabalho:");
  float proporcao;
  float Somatorio = 0;
  for(int i = 0; i < 4; i++) {
    if(leituraGraf[i] > 0) {
      if(i == 0) {
        proporcao = float(leituraGraf[0])/float(155);
      } else {
        proporcao = leituraGraf[i] - leituraGraf[i-1];
        Serial.println("Diferenca: " + String(proporcao));
        proporcao = float(proporcao)/float(155);
      }
      for(int y = 0; y <= 155; y++) {
        Somatorio+=proporcao;
        int posicao = map(Somatorio, 0, teto, 0, 255);
        graficoParametro.addValue(0, posicao);
      }
    }
  }
}
void Select_ImpPushCallback(void *ptr) {
  float leituraGraf[4] = {0,0,0,0};
  memset(buffer, 0, sizeof(buffer)); 
  IMPULSO_1.getText(buffer,sizeof(buffer));
  String Valor = String(buffer);
  leituraGraf[0] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  IMPULSO_2.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[1] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  IMPULSO_3.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[2] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  IMPULSO_4.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[3] = Valor.toFloat();
  long teto = 0;
  for(int i = 0; i < 4; i++) {
    teto = max(leituraGraf[i], teto);
  }
  teto + 5;
  page13.show();
  page = 13;
  TipoGrafico.setText("Impulso:");
  float proporcao;
  float Somatorio = 0;
  for(int i = 0; i < 4; i++) {
    if(leituraGraf[i] > 0) {
      if(i == 0) {
        proporcao = float(leituraGraf[0])/float(155);
      } else {
        proporcao = leituraGraf[i] - leituraGraf[i-1];
        Serial.println("Diferenca: " + String(proporcao));
        proporcao = float(proporcao)/float(155);
      }
      for(int y = 0; y <= 155; y++) {
        Somatorio+=proporcao;
        int posicao = map(Somatorio, 0, teto, 0, 255);
        graficoParametro.addValue(0, posicao);
      }
    }
  }
}
void Select_ResPushCallback(void *ptr) {
  float leituraGraf[4] = {0,0,0,0};
  memset(buffer, 0, sizeof(buffer)); 
  RESISTENCIA_1.getText(buffer,sizeof(buffer));
  String Valor = String(buffer);
  leituraGraf[0] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  RESISTENCIA_2.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[1] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  RESISTENCIA_3.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[2] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  RESISTENCIA_4.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[3] = Valor.toFloat();
  long teto = 0;
  for(int i = 0; i < 4; i++) {
    teto = max(leituraGraf[i], teto);
  }
  teto + 5;
  page13.show();
  page = 13;
  TipoGrafico.setText("Resistencia:");
  float proporcao;
  float Somatorio = 0;
  for(int i = 0; i < 4; i++) {
    if(leituraGraf[i] > 0) {
      if(i == 0) {
        proporcao = float(leituraGraf[0])/float(155);
      } else {
        proporcao = leituraGraf[i] - leituraGraf[i-1];
        Serial.println("Diferenca: " + String(proporcao));
        proporcao = float(proporcao)/float(155);
      }
      for(int y = 0; y <= 155; y++) {
        Somatorio+=proporcao;
        int posicao = map(Somatorio, 0, teto, 0, 255);
        graficoParametro.addValue(0, posicao);
      }
    }
  }
}
void Select_TraPushCallback(void *ptr) {
  float leituraGraf[4] = {0,0,0,0};
  memset(buffer, 0, sizeof(buffer)); 
  TRABALHO_1.getText(buffer,sizeof(buffer));
  String Valor = String(buffer);
  leituraGraf[0] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  TRABALHO_2.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[1] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  TRABALHO_3.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[2] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  TRABALHO_4.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[3] = Valor.toFloat();
  long teto = 0;
  for(int i = 0; i < 4; i++) {
    teto = max(leituraGraf[i], teto);
  }
  teto + 5;
  page13.show();
  page = 13;
  TipoGrafico.setText("Trabalho:");
  float proporcao;
  float Somatorio = 0;
  for(int i = 0; i < 4; i++) {
    if(leituraGraf[i] > 0) {
      if(i == 0) {
        proporcao = float(leituraGraf[0])/float(155);
      } else {
        proporcao = leituraGraf[i] - leituraGraf[i-1];
        Serial.println("Diferenca: " + String(proporcao));
        proporcao = float(proporcao)/float(155);
      }
      for(int y = 0; y <= 155; y++) {
        Somatorio+=proporcao;
        int posicao = map(Somatorio, 0, teto, 0, 255);
        graficoParametro.addValue(0, posicao);
      }
    }
  }
}
void Select_taxImpPushCallback(void *ptr) {
  float leituraGraf[4] = {0,0,0,0};
  memset(buffer, 0, sizeof(buffer)); 
  taxaImpulso_1.getText(buffer,sizeof(buffer));
  String Valor = String(buffer);
  leituraGraf[0] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  taxaImpulso_2.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[1] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  taxaImpulso_3.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[2] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  taxaImpulso_4.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[3] = Valor.toFloat();
  long teto = 0;
  for(int i = 0; i < 4; i++) {
    teto = max(leituraGraf[i], teto);
  }
  teto + 5;
  page13.show();
  page = 13;
  TipoGrafico.setText("TaxImpul:");
  float proporcao;
  float Somatorio = 0;
  for(int i = 0; i < 4; i++) {
    if(leituraGraf[i] > 0) {
      if(i == 0) {
        proporcao = float(leituraGraf[0])/float(155);
      } else {
        proporcao = leituraGraf[i] - leituraGraf[i-1];
        Serial.println("Diferenca: " + String(proporcao));
        proporcao = float(proporcao)/float(155);
      }
      for(int y = 0; y <= 155; y++) {
        Somatorio+=proporcao;
        int posicao = map(Somatorio, 0, teto, 0, 255);
        graficoParametro.addValue(0, posicao);
      }
    }
  }
}
void Select_taxResPushCallback(void *ptr) {
  float leituraGraf[4] = {0,0,0,0};
  memset(buffer, 0, sizeof(buffer)); 
  taxaResistencia_1.getText(buffer,sizeof(buffer));
  String Valor = String(buffer);
  leituraGraf[0] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  taxaResistencia_2.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[1] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  taxaResistencia_3.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[2] = Valor.toInt();
  memset(buffer, 0, sizeof(buffer)); 
  taxaResistencia_4.getText(buffer,sizeof(buffer));
  Valor = String(buffer);
  leituraGraf[3] = Valor.toFloat();
  long teto = 0;
  for(int i = 0; i < 4; i++) {
    teto = max(leituraGraf[i], teto);
  }
  teto + 5;
  page13.show();
  page = 13;
  TipoGrafico.setText("TaxResist:");
  float proporcao;
  float Somatorio = 0;
  for(int i = 0; i < 4; i++) {
    if(leituraGraf[i] > 0) {
      if(i == 0) {
        proporcao = float(leituraGraf[0])/float(155);
      } else {
        proporcao = leituraGraf[i] - leituraGraf[i-1];
        Serial.println("Diferenca: " + String(proporcao));
        proporcao = float(proporcao)/float(155);
      }
      for(int y = 0; y <= 155; y++) {
        Somatorio+=proporcao;
        int posicao = map(Somatorio, 0, teto, 0, 255);
        graficoParametro.addValue(0, posicao);
      }
    }
  }
}

void back11PushCallback(void *ptr) {
  page12.show();
  page = 12;
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ForcaMax[0].toCharArray(buffer,sizeof(buffer));
  fMax_1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ForcaMax[1].toCharArray(buffer,sizeof(buffer));
  fMax_2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ForcaMax[2].toCharArray(buffer,sizeof(buffer));
  fMax_3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ForcaMax[3].toCharArray(buffer,sizeof(buffer));
  fMax_4.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TFmax[0].toCharArray(buffer,sizeof(buffer));
  T1_1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TFmax[1].toCharArray(buffer,sizeof(buffer));
  T1_2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TFmax[2].toCharArray(buffer,sizeof(buffer));
  T1_3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TFmax[3].toCharArray(buffer,sizeof(buffer));
  T1_4.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TRes[0].toCharArray(buffer,sizeof(buffer));
  T2_1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TRes[1].toCharArray(buffer,sizeof(buffer));
  T2_2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TRes[2].toCharArray(buffer,sizeof(buffer));
  T2_3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TRes[3].toCharArray(buffer,sizeof(buffer));
  T2_4.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TempoTotal[0].toCharArray(buffer,sizeof(buffer));
  T3_1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TempoTotal[1].toCharArray(buffer,sizeof(buffer));
  T3_2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TempoTotal[2].toCharArray(buffer,sizeof(buffer));
  T3_3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TempoTotal[3].toCharArray(buffer,sizeof(buffer));
  T3_4.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ImPuLsO[0].toCharArray(buffer,sizeof(buffer));
  IMPULSO_1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ImPuLsO[1].toCharArray(buffer,sizeof(buffer));
  IMPULSO_2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ImPuLsO[2].toCharArray(buffer,sizeof(buffer));
  IMPULSO_3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ImPuLsO[3].toCharArray(buffer,sizeof(buffer));
  IMPULSO_4.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ReSiStEnCia[0].toCharArray(buffer,sizeof(buffer));
  RESISTENCIA_1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ReSiStEnCia[1].toCharArray(buffer,sizeof(buffer));
  RESISTENCIA_2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ReSiStEnCia[2].toCharArray(buffer,sizeof(buffer));
  RESISTENCIA_3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  ReSiStEnCia[3].toCharArray(buffer,sizeof(buffer));
  RESISTENCIA_4.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TrAbAlHo[0].toCharArray(buffer,sizeof(buffer));
  TRABALHO_1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TrAbAlHo[1].toCharArray(buffer,sizeof(buffer));
  TRABALHO_2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TrAbAlHo[2].toCharArray(buffer,sizeof(buffer));
  TRABALHO_3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  TrAbAlHo[3].toCharArray(buffer,sizeof(buffer));
  TRABALHO_4.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  tAxRes[0].toCharArray(buffer,sizeof(buffer));
  taxaResistencia_1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  tAxRes[1].toCharArray(buffer,sizeof(buffer));
  taxaResistencia_2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  tAxRes[2].toCharArray(buffer,sizeof(buffer));
  taxaResistencia_3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  tAxRes[3].toCharArray(buffer,sizeof(buffer));
  taxaResistencia_4.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  tAxImp[0].toCharArray(buffer,sizeof(buffer));
  taxaImpulso_1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  tAxImp[1].toCharArray(buffer,sizeof(buffer));
  taxaImpulso_2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  tAxImp[2].toCharArray(buffer,sizeof(buffer));
  taxaImpulso_3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  tAxImp[3].toCharArray(buffer,sizeof(buffer));
  taxaImpulso_4.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  NameData[0].toCharArray(buffer,sizeof(buffer));
  NomeData1.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  NameData[1].toCharArray(buffer,sizeof(buffer));
  NomeData2.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  NameData[2].toCharArray(buffer,sizeof(buffer));
  NomeData3.setText(buffer); //envia valor para
  memset(buffer, 0, sizeof(buffer));  //armazena os valores no buffer
  NameData[3].toCharArray(buffer,sizeof(buffer));
  NomeData4.setText(buffer); //envia valor para
}

void setup() {

  Serial.begin(115200);

  //Declara os pinos a serem usados, OUTPUT envia os dados, INPUT os recebe
  pinMode(PinBuzzer, OUTPUT); 
  pinMode(PinSensor, INPUT);  

  //Procedimento necessário para todas as funções que ocorrerão ao toque de um botão
  //objetoTouch.attachPush(nomeDaFunçãoDoObjetoTouch, &objetoTouch)
                //^Isto depende se o objeto respondera ao apertar ou soltar
  nexInit();
  menuInicial.attachPush(menuInicialPushCallback, &menuInicial);
  Dinamometro.attachPush(DinamometroPushCallback, &Dinamometro);
  Analisador.attachPush(AnalisadorPushCallback, &Analisador);
  back1.attachPush(back1PushCallback, &back1);
  zerarDinam.attachPush(zerarDinamPushCallback, &zerarDinam);
  iniciarDinam.attachPush(iniciarDinamPushCallback, &iniciarDinam);
  Novo.attachPush(NovoPushCallback, &Novo);
  Cadastrado.attachPush(CadastradoPushCallback, &Cadastrado);
  RESETAR.attachPush(RESETARPushCallback, &RESETAR);
  back2.attachPush(back2PushCallback, &back2);
  sair.attachPush(sairPushCallback, &sair);
  Medicao.attachPush(MedicaoPushCallback, &Medicao);
  Historico.attachPush(HistoricoPushCallback, &Historico);
  back3.attachPush(back3PushCallback, &back3);
  back4.attachPush(back4PushCallback, &back4);
  Direita.attachPush(DireitaPushCallback, &Direita);
  Esquerda.attachPush(EsquerdaPushCallback, &Esquerda);
  back5.attachPush(back5PushCallback, &back5);
  bZErar.attachPush(bZErarPushCallback, &bZErar);
  back6.attachPush(back6PushCallback, &back6);
  refazer.attachPush(refazerPushCallback, &refazer);
  back7.attachPush(back7PushCallback, &back7);
  back8.attachPush(back8PushCallback, &back8);
  esq.attachPush(esqPushCallback, &esq);
  dir.attachPush(dirPushCallback, &dir);
  back9.attachPush(back9PushCallback, &back9);
  Reset.attachPush(ResetPushCallback, &Reset);
  graficoTabela.attachPush(graficoTabelaPushCallback, &graficoTabela);
  consulta1.attachPush(consulta1PushCallback, &consulta1);
  consulta2.attachPush(consulta2PushCallback, &consulta2);
  consulta3.attachPush(consulta3PushCallback, &consulta3);
  consulta4.attachPush(consulta4PushCallback, &consulta4);
  consulta5.attachPush(consulta5PushCallback, &consulta5);
  consulta6.attachPush(consulta6PushCallback, &consulta6);
  consulta7.attachPush(consulta7PushCallback, &consulta7);
  consulta8.attachPush(consulta8PushCallback, &consulta8);
  consulta9.attachPush(consulta9PushCallback, &consulta9);
  consulta10.attachPush(consulta10PushCallback, &consulta10);
  back10.attachPush(back10PushCallback, &back10);
  Delete.attachPush(DeletePushCallback, &Delete);
  Select_FMAX.attachPush(Select_FMAXPushCallback, &Select_FMAX);
  Select_T1.attachPush(Select_T1PushCallback, &Select_T1);   
  Select_T2.attachPush(Select_T2PushCallback, &Select_T2);    
  Select_T3.attachPush(Select_T3PushCallback, &Select_T3);
  Select_Imp.attachPush(Select_ImpPushCallback, &Select_Imp);
  Select_Res.attachPush(Select_ResPushCallback, &Select_Res);         
  Select_Tra.attachPush(Select_TraPushCallback, &Select_Tra);           
  Select_taxImp.attachPush(Select_taxImpPushCallback, &Select_taxImp);        
  Select_taxRes.attachPush(Select_taxResPushCallback, &Select_taxRes);         
  back11.attachPush(back11PushCallback, &back11);
  
  ADCSRA &= ~PS_128;  //Procedimento para aumentar a velocidade da leitura da porta analógica, não sei como funciona
  ADCSRA != PS_128;   

  page = 0; //Nextion inicializa na pagina 0
}

void loop() {
  switch(page) {
  case 0:
    nexLoop(Page0);
    while (!Serial) {
    }
    if (SD.begin(chipSelect)) {
      CartaoSD.setText("Conectado");
    }
    else {
      CartaoSD.setText("Nao Conectado");
    }
    break;
  case 1:
    nexLoop(Page1);
    break;
  case 2:
    nexLoop(Page2);
    break;
  case 3:
    nexLoop(Page3);
    break;
  case 4:
    nexLoop(Page4);
    break;
  case 5:
    nexLoop(Page5);
    break;
  case 6:
    nexLoop(Page6);  
    break;
  case 7:
    nexLoop(Page7);
    break;
  case 8:
    nexLoop(Page8);
    break;
  case 9:
    nexLoop(Page9);
    break;
  case 10: 
    nexLoop(Page10);
    break;
  case 11:
    nexLoop(Page11);
    break;
  case 12:
    nexLoop(Page12);
    break;
  case 13:
    nexLoop(Page13);
    break;
}
delay(100);
}
