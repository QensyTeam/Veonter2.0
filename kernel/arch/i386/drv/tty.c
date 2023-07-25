#include <kernel/kernel.h>

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

char* title = 
"\n\t\t\t\t##  ## #####  ####  ##  ## ###### ##### ##### \n\
\t\t\t\t##  ## ##    ##  ## ### ##   ##   ##    ##  ##\n\
\t\t\t\t##  ## ####  ##  ## ## ###   ##   ####  ##### \n\
\t\t\t\t ####  ##    ##  ## ##  ##   ##   ##    ##  ##\n\
\t\t\t\t  ##   #####  ####  ##  ##   ##   ##### ##  ##\n\n";

void terminal_startscreen() {
    terminal_set_color(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BROWN));
    printf(title);
}



void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BROWN);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
	terminal_enable_cursor(14, 15); // Включаем курсор с формой нижней половины символа
	terminal_update_cursor(terminal_column, terminal_row); // Обновляем позицию курсора
	check();
	printf("Terminal initialization completed successfully!\n");
}

void check(void) {
	terminal_set_color(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BROWN));
	printf("[ ");
	terminal_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BROWN));
	printf("OK");
	terminal_set_color(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BROWN));
	printf(" ] - ");
}

// Функция для получения текущего столбца терминала
size_t terminal_getcolumn(void) {
 	return terminal_column;
}

size_t terminal_getRow(){
  return terminal_row;
}

void terminal_set_color(uint8_t color) {
	terminal_color = color;
	terminal_update();
	terminal_enable_cursor(14, 15); // Включаем курсор с формой нижней половины символа
	terminal_update_cursor(terminal_column, terminal_row); // Обновляем позицию курсора
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_update(void) {
	memcpy(VGA_MEMORY, terminal_buffer, VGA_WIDTH * VGA_HEIGHT * sizeof(uint16_t));
}


void terminal_clear_line(size_t y)
{ 
	size_t x = 0;
	while(x < VGA_WIDTH){
		terminal_putentryat(' ', terminal_color, x, y);
		x++;
	}
}

void terminal_clearscreen(void)
{
	size_t y = 0;
	while(y < VGA_HEIGHT){
		terminal_clear_line(y);
		y++;
	}
	terminal_row = 0;
	terminal_column = 0;
}

/* Scrolls the terminal based on where the cursor is. */
void terminal_scroll(void){
	uint16_t temp;
	
	/* Row 25 is the end, this means we need to scroll up */
	if(terminal_row >= 25)
	{
		
		/* Move the current text chunk that makes up the screen
		back in the buffer by a line */
		temp = terminal_row - 25 + 1;
		memcpy (VGA_MEMORY, VGA_MEMORY + temp * 80, (25 - temp) * 80 * 2);

		/* Finally, we set the chunk of memory that occupies
		the last line of text to our 'blank' character */
		for(int i = 0; i < 80; i++)
		{
			terminal_putentryat(' ', terminal_color, i, 24);
			
		}
		terminal_row = 25 - 1;
		
	}
}

void terminal_putchar(char c)
{
    if (c == '\b') { // Если символ - backspace
        if (terminal_column > 0) { // Проверяем, что курсор не находится в крайнем левом столбце
            terminal_column--; // Передвигаем курсор на предыдущую позицию
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row); // Стираем символ на экране
            terminal_update_cursor(terminal_column, terminal_row); // Обновляем позицию курсора на экране
        } else if (terminal_row > 0) { // Если курсор находится в крайнем левом столбце текущей строки, но не в первой строке
            terminal_row--; // Перемещаем курсор на предыдущую строку
            terminal_column = VGA_WIDTH - 1; // Устанавливаем курсор в крайний правый столбец предыдущей строки
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row); // Стираем символ на экране
            terminal_update_cursor(terminal_column, terminal_row); // Обновляем позицию курсора на экране
        }
        // Удаляем символ из памяти только если курсор был перемещен
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
	}
 	else if(c == '\n')
	{
 		terminal_column = 0;
 		terminal_row++;
 	}
	else if(c == '\t') 
	{
		terminal_putchar(' ');
		terminal_putchar(' ');
		terminal_putchar(' ');
		terminal_putchar(' ');
	}
	else if (c == '\v') {
    	// Обработка символа '\v'
    	terminal_row++; // Увеличиваем номер строки
		if (terminal_row >= VGA_HEIGHT) {
			// Если достигли конца экрана, сбрасываем номер строки и производим прокрутку
			terminal_row = VGA_HEIGHT - 1;
			terminal_scroll();
    	}
    	terminal_column = 0; // Сбрасываем номер столбца
	} else if (c == '\f') {
		// Обработка символа '\f'
		terminal_clearscreen(); // Очищаем экран
	} else if (c == '\r') {
		// Обработка символа '\r'
		terminal_column = 0; // Сбрасываем номер столбца
	}
	else 
	{
 		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if(++terminal_column == VGA_WIDTH)
		{
			terminal_column = 0;
			terminal_row++;
		}
 	} 
	terminal_scroll();
 	terminal_update_cursor(terminal_column, terminal_row);
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

// Функция для чтения строки из терминала и сохранения ее в буфер
void terminal_getstring(char* buffer) {
 	size_t index = 0; // Индекс в буфере
 	for (size_t y = 0; y < VGA_HEIGHT; y++) {
  		for (size_t x = 0; x < VGA_WIDTH; x++) {
   			// Получаем символ из терминала по координатам
   			uint16_t entry = terminal_buffer[y * VGA_WIDTH + x];
   			char c = entry & 0xFF;
   			if (c == '\0') {
    			// Если символ - нуль-терминатор, то заканчиваем чтение
    			buffer[index] = '\0';
    			return;
   			} else {
    			// Иначе добавляем символ в буфер
    			buffer[index] = c;
    			index++;
   			}
  		}
 	}
 	// Добавляем нуль-терминатор в конец буфера
 	buffer[index] = '\0';
}


char terminal_getchar(void) {
    char c = '\0';
    while (c == '\0') {
        terminal_getstring(&c);
    }
    return c;
}
