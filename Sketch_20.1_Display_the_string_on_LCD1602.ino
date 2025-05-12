#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* password = "mcmaster4862";
const char* ssid = "Perry the platypus";
const char* mqtt_server = "35.227.189.114";  // GCP IP
#define SDA 14                    //Define SDA pins
#define SCL 13                    //Define SCL pins

// MQTT topics
const char* topic_playerX = "player1/move";
const char* topic_playerO = "player2/move";
const char* topic_feedback = "tictactoe/feedback";
const char* topic_gameStatus = "tictactoe/gameStatus";
const char* topic_board = "tictactoe/board";
const char* topic_gameAvailable = "tictactoe/gameAvailable";
const char* topic_gamesPlayed = "tictactoe/gamesPlayed";

// Wi-Fi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust address if necessary

// Game variables
char board[9];
char currentPlayer = 'X';
int xWins = 0;
int oWins = 0;
int draws = 0;
int gamesPlayed = 0;
bool gameInProgress = true;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);           // attach the IIC pin
    if (!i2CAddrTest(0x27)) {
      lcd = LiquidCrystal_I2C(0x3F, 16, 2);
    }
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Connect to MQTT broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(topic_playerX);
      client.subscribe(topic_playerO);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }

  // Initialize game
  initializeBoard();
  displayBoard();
  publishGameAvailability();
}

void loop() {
  client.loop();
  publishBoard();
}

void initializeBoard() {
  for (int i = 0; i < 9; i++) {
    board[i] = ' ';
  }
  gameInProgress = true;
  currentPlayer = 'X';
  publishGameAvailability();
  publishBoard();
}

void displayBoard() {
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int i = 0; i < 9; i++) {
    lcd.print(board[i]);
  }
  lcd.setCursor(0, 1);
  lcd.print("X:");
  lcd.print(xWins);
  lcd.print(" O:");
  lcd.print(oWins);
  lcd.print(" D:");
  lcd.print(draws);
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (!gameInProgress) return;

  // Convert payload to string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  // Parse move
  int move = atoi(message);
  if (move < 1 || move > 9) {
    client.publish(topic_feedback, "Invalid move: Out of range");
    return;
  }
  int index = move - 1;

  // Determine player
  char player = (strcmp(topic, topic_playerX) == 0) ? 'X' : 'O';

  // Validate move
  if (board[index] != ' ') {
    client.publish(topic_feedback, "Invalid move: Cell already occupied");
    return;
  }

  // Make move
  board[index] = player;
  displayBoard();
  publishBoard();

  // Check for win
  if (checkWin(player)) {
    String winMessage = String(player) + " wins";
    client.publish(topic_gameStatus, winMessage.c_str());
    if (player == 'X') xWins++;
    else oWins++;
    endGame();
    return;
  }

  // Check for draw
  if (isBoardFull()) {
    client.publish(topic_gameStatus, "Draw");
    draws++;
    endGame();
    return;
  }

  client.publish(topic_gameStatus, "Ongoing");

  // Switch player
  currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
}

bool checkWin(char player) {
  int winCombos[8][3] = {
    {0,1,2}, {3,4,5}, {6,7,8}, // Rows
    {0,3,6}, {1,4,7}, {2,5,8}, // Columns
    {0,4,8}, {2,4,6}           // Diagonals
  };
  for (int i = 0; i < 8; i++) {
    if (board[winCombos[i][0]] == player &&
        board[winCombos[i][1]] == player &&
        board[winCombos[i][2]] == player)
      return true;
  }
  return false;
}

bool isBoardFull() {
  for (int i = 0; i < 9; i++)
    if (board[i] == ' ')
      return false;
  return true;
}

void endGame() {
  gamesPlayed++;
  client.publish(topic_gamesPlayed, String(gamesPlayed).c_str());
  gameInProgress = false;
  publishGameAvailability();
  delay(1000); // Delay before starting a new game
  if (gamesPlayed < 100) {
    initializeBoard();
    displayBoard();
    publishGameAvailability();
  }
  if (gamesPlayed == 100) {
    for (int i = 0; i < 9; i++) {
    board[i] = ' ';
    }
  displayBoard();
  }
}

void publishGameAvailability() {
  client.publish(topic_gameAvailable, gameInProgress ? "1" : "0");
}

void publishBoard() {
  String boardState = "";
  for (int i = 0; i < 9; i++) {
    boardState += board[i];
  }
  char payload[10]; // 9 characters + null terminator
  boardState.toCharArray(payload, sizeof(payload));

  client.publish(topic_board, payload);
}

bool i2CAddrTest(uint8_t addr) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}