ConsoleReader *consoleReader = new ConsoleReader();
connect (consoleReader, SIGNAL (KeyPressed(char)), this, SLOT(OnConsoleKeyPressed(char)));
consoleReader->start();
