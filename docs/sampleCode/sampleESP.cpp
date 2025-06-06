void setup()
{
    pinMode(D5, OUTPUT); // GPIO14
}

void loop()
{
    digitalWrite(D5, HIGH); // Bật LED
    delay(1000);
    digitalWrite(D5, LOW); // Tắt LED
    delay(1000);
}
