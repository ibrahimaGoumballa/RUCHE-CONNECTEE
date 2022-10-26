void setup() {
Serial.begin(9600);
 

}

void loop() {


const float TensionMin = 2.8; //tension min
const float TensionMax = 4.2; //tension max

int valeur = analogRead(A0); // Mesure la tension sur la broche A0
float tension = valeur * (3.0 / 929.0); // Transforme la mesure (nombre entier) en tension via un produit en croix, 929 = Valeur max pour 3V du convertisseur
float Pourcent=(tension/3)*100; 

Serial.print(tension);
Serial.println("volt");
Serial.print(Pourcent);
Serial.println("%");
delay(3000);
}
