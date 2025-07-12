#include <Ps3Controller.h>
#include <M5Core2.h>

int player = 0;
int battery = 0;

void notify()
{
    //--- Digital cross/square/triangle/circle button events ---
    if( Ps3.event.button_down.cross )
        M5.Lcd.println("Started pressing the cross button");
    if( Ps3.event.button_up.cross )
        M5.Lcd.println("Released the cross button");

    if( Ps3.event.button_down.square )
        M5.Lcd.println("Started pressing the square button");
    if( Ps3.event.button_up.square )
        M5.Lcd.println("Released the square button");

    if( Ps3.event.button_down.triangle )
        M5.Lcd.println("Started pressing the triangle button");
    if( Ps3.event.button_up.triangle )
        M5.Lcd.println("Released the triangle button");

    if( Ps3.event.button_down.circle )
        M5.Lcd.println("Started pressing the circle button");
    if( Ps3.event.button_up.circle )
        M5.Lcd.println("Released the circle button");

    //--------------- Digital D-pad button events --------------
    if( Ps3.event.button_down.up )
        M5.Lcd.println("Started pressing the up button");
    if( Ps3.event.button_up.up )
        M5.Lcd.println("Released the up button");

    if( Ps3.event.button_down.right )
        M5.Lcd.println("Started pressing the right button");
    if( Ps3.event.button_up.right )
        M5.Lcd.println("Released the right button");

    if( Ps3.event.button_down.down )
        M5.Lcd.println("Started pressing the down button");
    if( Ps3.event.button_up.down )
        M5.Lcd.println("Released the down button");

    if( Ps3.event.button_down.left )
        M5.Lcd.println("Started pressing the left button");
    if( Ps3.event.button_up.left )
        M5.Lcd.println("Released the left button");

    //------------- Digital shoulder button events -------------
    if( Ps3.event.button_down.l1 )
        M5.Lcd.println("Started pressing the left shoulder button");
    if( Ps3.event.button_up.l1 )
        M5.Lcd.println("Released the left shoulder button");

    if( Ps3.event.button_down.r1 )
        M5.Lcd.println("Started pressing the right shoulder button");
    if( Ps3.event.button_up.r1 )
        M5.Lcd.println("Released the right shoulder button");

    //-------------- Digital trigger button events -------------
    if( Ps3.event.button_down.l2 )
        M5.Lcd.println("Started pressing the left trigger button");
    if( Ps3.event.button_up.l2 )
        M5.Lcd.println("Released the left trigger button");

    if( Ps3.event.button_down.r2 )
        M5.Lcd.println("Started pressing the right trigger button");
    if( Ps3.event.button_up.r2 )
        M5.Lcd.println("Released the right trigger button");

    //--------------- Digital stick button events --------------
    if( Ps3.event.button_down.l3 )
        M5.Lcd.println("Started pressing the left stick button");
    if( Ps3.event.button_up.l3 )
        M5.Lcd.println("Released the left stick button");

    if( Ps3.event.button_down.r3 )
        M5.Lcd.println("Started pressing the right stick button");
    if( Ps3.event.button_up.r3 )
        M5.Lcd.println("Released the right stick button");

    //---------- Digital select/start/ps button events ---------
    if( Ps3.event.button_down.select )
        M5.Lcd.println("Started pressing the select button");
    if( Ps3.event.button_up.select )
        M5.Lcd.println("Released the select button");

    if( Ps3.event.button_down.start )
        M5.Lcd.println("Started pressing the start button");
    if( Ps3.event.button_up.start )
        M5.Lcd.println("Released the start button");

    if( Ps3.event.button_down.ps )
        M5.Lcd.println("Started pressing the Playstation button");
    if( Ps3.event.button_up.ps )
        M5.Lcd.println("Released the Playstation button");


    //---------------- Analog stick value events ---------------
   if( abs(Ps3.event.analog_changed.stick.lx) + abs(Ps3.event.analog_changed.stick.ly) > 2 ){
       M5.Lcd.print("Moved the left stick:");
       M5.Lcd.print(" x="); M5.Lcd.print(Ps3.data.analog.stick.lx, DEC);
       M5.Lcd.print(" y="); M5.Lcd.print(Ps3.data.analog.stick.ly, DEC);
       M5.Lcd.println();
    }

   if( abs(Ps3.event.analog_changed.stick.rx) + abs(Ps3.event.analog_changed.stick.ry) > 2 ){
       M5.Lcd.print("Moved the right stick:");
       M5.Lcd.print(" x="); M5.Lcd.print(Ps3.data.analog.stick.rx, DEC);
       M5.Lcd.print(" y="); M5.Lcd.print(Ps3.data.analog.stick.ry, DEC);
       M5.Lcd.println();
   }

   //--------------- Analog D-pad button events ----------------
   if( abs(Ps3.event.analog_changed.button.up) ){
       M5.Lcd.print("Pressing the up button: ");
       M5.Lcd.println(Ps3.data.analog.button.up, DEC);
   }

   if( abs(Ps3.event.analog_changed.button.right) ){
       M5.Lcd.print("Pressing the right button: ");
       M5.Lcd.println(Ps3.data.analog.button.right, DEC);
   }

   if( abs(Ps3.event.analog_changed.button.down) ){
       M5.Lcd.print("Pressing the down button: ");
       M5.Lcd.println(Ps3.data.analog.button.down, DEC);
   }

   if( abs(Ps3.event.analog_changed.button.left) ){
       M5.Lcd.print("Pressing the left button: ");
       M5.Lcd.println(Ps3.data.analog.button.left, DEC);
   }

   //---------- Analog shoulder/trigger button events ----------
   if( abs(Ps3.event.analog_changed.button.l1)){
       M5.Lcd.print("Pressing the left shoulder button: ");
       M5.Lcd.println(Ps3.data.analog.button.l1, DEC);
   }

   if( abs(Ps3.event.analog_changed.button.r1) ){
       M5.Lcd.print("Pressing the right shoulder button: ");
       M5.Lcd.println(Ps3.data.analog.button.r1, DEC);
   }

   if( abs(Ps3.event.analog_changed.button.l2) ){
       M5.Lcd.print("Pressing the left trigger button: ");
       M5.Lcd.println(Ps3.data.analog.button.l2, DEC);
   }

   if( abs(Ps3.event.analog_changed.button.r2) ){
       M5.Lcd.print("Pressing the right trigger button: ");
       M5.Lcd.println(Ps3.data.analog.button.r2, DEC);
   }

   //---- Analog cross/square/triangle/circle button events ----
   if( abs(Ps3.event.analog_changed.button.triangle)){
       M5.Lcd.print("Pressing the triangle button: ");
       M5.Lcd.println(Ps3.data.analog.button.triangle, DEC);
   }

   if( abs(Ps3.event.analog_changed.button.circle) ){
       M5.Lcd.print("Pressing the circle button: ");
       M5.Lcd.println(Ps3.data.analog.button.circle, DEC);
   }

   if( abs(Ps3.event.analog_changed.button.cross) ){
       M5.Lcd.print("Pressing the cross button: ");
       M5.Lcd.println(Ps3.data.analog.button.cross, DEC);
   }

   if( abs(Ps3.event.analog_changed.button.square) ){
       M5.Lcd.print("Pressing the square button: ");
       M5.Lcd.println(Ps3.data.analog.button.square, DEC);
   }

   //---------------------- Battery events ---------------------
    if( battery != Ps3.data.status.battery ){
        battery = Ps3.data.status.battery;
        M5.Lcd.print("The controller battery is ");
        if( battery == ps3_status_battery_charging )      M5.Lcd.println("charging");
        else if( battery == ps3_status_battery_full )     M5.Lcd.println("FULL");
        else if( battery == ps3_status_battery_high )     M5.Lcd.println("HIGH");
        else if( battery == ps3_status_battery_low)       M5.Lcd.println("LOW");
        else if( battery == ps3_status_battery_dying )    M5.Lcd.println("DYING");
        else if( battery == ps3_status_battery_shutdown ) M5.Lcd.println("SHUTDOWN");
        else M5.Lcd.println("UNDEFINED");
    }

}

void onConnect(){
    M5.Lcd.println("Connected.");
}

void setup()
{
  Serial.begin(115200);
  M5.begin(true, true, true, true);
  M5.Lcd.setTextSize(1);
  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.begin("78:21:84:93:5C:EC");

  M5.Lcd.println("Ready.");
}

void loop()
{
    if(!Ps3.isConnected())
        return;

    //-------------------- Player LEDs -------------------
    // M5.Lcd.print("Setting LEDs to player "); M5.Lcd.println(player, DEC);
    Ps3.setPlayer(player);

    player += 1;
    if(player > 10) player = 0;


    //------ Digital cross/square/triangle/circle buttons ------
    if( Ps3.data.button.cross && Ps3.data.button.down )
        M5.Lcd.println("Pressing both the down and cross buttons");
    if( Ps3.data.button.square && Ps3.data.button.left )
        M5.Lcd.println("Pressing both the square and left buttons");
    if( Ps3.data.button.triangle && Ps3.data.button.up )
        M5.Lcd.println("Pressing both the triangle and up buttons");
    if( Ps3.data.button.circle && Ps3.data.button.right )
        M5.Lcd.println("Pressing both the circle and right buttons");

    if( Ps3.data.button.l1 && Ps3.data.button.r1 )
        M5.Lcd.println("Pressing both the left and right bumper buttons");
    if( Ps3.data.button.l2 && Ps3.data.button.r2 )
        M5.Lcd.println("Pressing both the left and right trigger buttons");
    if( Ps3.data.button.l3 && Ps3.data.button.r3 )
        M5.Lcd.println("Pressing both the left and right stick buttons");
    if( Ps3.data.button.select && Ps3.data.button.start )
        M5.Lcd.println("Pressing both the select and start buttons");

    delay(2000);
}
