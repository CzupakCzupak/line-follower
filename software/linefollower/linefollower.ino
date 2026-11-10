#define ML_IN1  7
#define ML_IN2  8
#define ML_PWM  9

#define MR_IN1  5
#define MR_IN2  6
#define MR_PWM  10

#define S0  A0
#define S1  A1
#define S2  A2
#define S3  A3
#define S4  A4
#define POT A5

#define LED0  0
#define LED1  1
#define LED2  2
#define LED3  3
#define LED4  4

#define BUTTON  11

#define BASE_SPEED  215
#define MAX_SPEED   255
#define MIN_SPEED  -240

float Kp = 108.0;
float Kd = 6800.0;

float lastError = 0;
int threshold = 800;

void setup() {
  pinMode(ML_PWM, OUTPUT); digitalWrite(ML_PWM, LOW);
  pinMode(MR_PWM, OUTPUT); digitalWrite(MR_PWM, LOW);
  pinMode(ML_IN1, OUTPUT); digitalWrite(ML_IN1, LOW);
  pinMode(ML_IN2, OUTPUT); digitalWrite(ML_IN2, LOW);
  pinMode(MR_IN1, OUTPUT); digitalWrite(MR_IN1, LOW);
  pinMode(MR_IN2, OUTPUT); digitalWrite(MR_IN2, LOW);

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  pinMode(BUTTON, INPUT_PULLUP);

  bool potMoved = false;
  int startPot = analogRead(POT);

  while (true) {
    int potVal = analogRead(POT);

    if (abs(potVal - startPot) > 30) potMoved = true;

    if (potMoved) {
      threshold = map(potVal, 0, 1023, 300, 950);
    }

    int level = map(threshold, 300, 950, 1, 5);
    for (int i = 0; i < 5; i++) {
      digitalWrite(i, i < level ? LOW : HIGH);
    }

    if (digitalRead(BUTTON) == LOW) {
      delay(50);
      while (digitalRead(BUTTON) == LOW);
      delay(50);
      break;
    }

    delay(50);
  }

  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < 5; i++) digitalWrite(i, LOW);
    delay(150);
    for (int i = 0; i < 5; i++) digitalWrite(i, HIGH);
    delay(150);
  }

  while (digitalRead(BUTTON) == HIGH) {
    digitalWrite(LED2, LOW);
    delay(200);
    digitalWrite(LED2, HIGH);
    delay(200);
  }

  delay(50);
  while (digitalRead(BUTTON) == LOW);
  digitalWrite(LED0, LOW); delay(500);
  digitalWrite(LED1, LOW); delay(500);
  digitalWrite(LED2, LOW); delay(500);
  digitalWrite(LED3, LOW); delay(500);

  for (int i = 0; i <= 4; i++) digitalWrite(i, HIGH);
}

void motorL(int pwm) {
  if (pwm >= 0) {
    digitalWrite(ML_IN1, LOW);
    digitalWrite(ML_IN2, HIGH);
  } else {
    digitalWrite(ML_IN1, HIGH);
    digitalWrite(ML_IN2, LOW);
    pwm = -pwm;
  }
  if (pwm > 255) pwm = 255;
  analogWrite(ML_PWM, pwm);
}

void motorR(int pwm) {
  if (pwm >= 0) {
    digitalWrite(MR_IN1, LOW);
    digitalWrite(MR_IN2, HIGH);
  } else {
    digitalWrite(MR_IN1, HIGH);
    digitalWrite(MR_IN2, LOW);
    pwm = -pwm;
  }
  if (pwm > 255) pwm = 255;
  analogWrite(MR_PWM, pwm);
}

void loop() {
  int v[5];
  v[0] = analogRead(S0);
  v[1] = analogRead(S1);
  v[2] = analogRead(S2);
  v[3] = analogRead(S3);
  v[4] = analogRead(S4);

  bool b[5];
  for (int i = 0; i < 5; i++) b[i] = v[i] > threshold;

  int leds[5] = {LED0, LED1, LED2, LED3, LED4};
  for (int i = 0; i < 5; i++) {
    digitalWrite(leds[i], b[i] ? LOW : HIGH);
  }

  int weights[5] = {-2, -1, 0, 1, 2};
  long sum = 0;
  int count = 0;
  for (int i = 0; i < 5; i++) {
    if (b[i]) {
      sum += weights[i];
      count++;
    }
  }

  static float error = 0;

  if (count > 0) {
    error = (float)sum / count;
  } else {
    if (lastError > 0) error = 2.5;
    else if (lastError < 0) error = -2.5;
  }

  float derivative = error - lastError;
  float correction = Kp * error + Kd * derivative;
  lastError = error;

  int leftPwm  = BASE_SPEED + correction;
  int rightPwm = BASE_SPEED - correction;

  if (leftPwm  > MAX_SPEED) leftPwm  = MAX_SPEED;
  if (rightPwm > MAX_SPEED) rightPwm = MAX_SPEED;
  if (leftPwm  < MIN_SPEED) leftPwm  = MIN_SPEED;
  if (rightPwm < MIN_SPEED) rightPwm = MIN_SPEED;

  motorL(leftPwm);
  motorR(rightPwm);
}