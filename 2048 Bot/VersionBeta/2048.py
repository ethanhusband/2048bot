# GUI and game running logic written by andersqiu

from __future__ import print_function

from bot import Bot
from grid import Grid
try:
    import tkinter as tk # For Python 3
    import tkinter.messagebox as messagebox
except:
    import Tkinter as tk # For Python 2
    import tkMessageBox as messagebox
import sys
import time



class GamePanel:
    '''The GUI view class of the 2048 game showing via tkinter.'''
    CELL_PADDING = 10
    BACKGROUND_COLOR = '#92877d'
    EMPTY_CELL_COLOR = '#9e948a'
    CELL_BACKGROUND_COLOR_DICT = {
        '2': '#eee4da',
        '4': '#ede0c8',
        '8': '#f2b179',
        '16': '#f59563',
        '32': '#f67c5f',
        '64': '#f65e3b',
        '128': '#edcf72',
        '256': '#edcc61',
        '512': '#edc850',
        '1024': '#edc53f',
        '2048': '#edc22e',
        'beyond': '#3c3a32'
    }
    CELL_COLOR_DICT = {
        '2': '#776e65',
        '4': '#776e65',
        '8': '#f9f6f2',
        '16': '#f9f6f2',
        '32': '#f9f6f2',
        '64': '#f9f6f2',
        '128': '#f9f6f2',
        '256': '#f9f6f2',
        '512': '#f9f6f2',
        '1024': '#f9f6f2',
        '2048': '#f9f6f2',
        'beyond': '#f9f6f2'
    }
    FONT = ('Verdana', 24, 'bold')
    UP_KEYS = ('w', 'W', 'Up')
    LEFT_KEYS = ('a', 'A', 'Left')
    DOWN_KEYS = ('s', 'S', 'Down')
    RIGHT_KEYS = ('d', 'D', 'Right')
    ALL_KEYS = UP_KEYS + LEFT_KEYS + DOWN_KEYS + RIGHT_KEYS

    def __init__(self, grid):
        self.grid = grid
        self.root = tk.Tk()
        if sys.platform == 'win32':
            self.root.iconbitmap('2048.ico')
        self.root.title('2048')
        self.root.resizable(False, False)
        self.background = tk.Frame(self.root, bg=GamePanel.BACKGROUND_COLOR)
        self.cell_labels = []
        for i in range(self.grid.size):
            row_labels = []
            for j in range(self.grid.size):
                label = tk.Label(self.background, text='',
                                 bg=GamePanel.EMPTY_CELL_COLOR,
                                 justify=tk.CENTER, font=GamePanel.FONT,
                                 width=4, height=2)
                label.grid(row=i, column=j, padx=10, pady=10)
                row_labels.append(label)
            self.cell_labels.append(row_labels)
        self.background.pack(side=tk.TOP)

    def paint(self):
        for i in range(self.grid.size):
            for j in range(self.grid.size):
                if self.grid.cells[i][j] == 0:
                    self.cell_labels[i][j].configure(
                         text='',
                         bg=GamePanel.EMPTY_CELL_COLOR)
                else:
                    cell_text = str(self.grid.cells[i][j])
                    if self.grid.cells[i][j] > 2048:
                        bg_color = GamePanel.CELL_BACKGROUND_COLOR_DICT.get('beyond')
                        fg_color = GamePanel.CELL_COLOR_DICT.get('beyond')
                    else:
                        bg_color = GamePanel.CELL_BACKGROUND_COLOR_DICT.get(cell_text)
                        fg_color = GamePanel.CELL_COLOR_DICT.get(cell_text)
                    self.cell_labels[i][j].configure(
                        text=cell_text,
                        bg=bg_color, fg=fg_color)

class Game:
    '''The main game class which is the controller of the whole game.'''
    def __init__(self, grid, panel, uses_ai):
        self.grid = grid
        self.panel = panel
        self.start_cells_num = 2
        self.over = False
        self.won = False
        self.keep_playing = False
        self.uses_ai = uses_ai

    def is_game_terminated(self):
        return self.over or (self.won and (not self.keep_playing))

    def start(self):
        self.add_start_cells()
        if (self.uses_ai):
            self.ai_handler()
        else:
            self.panel.paint()
            self.panel.root.bind('<Key>', self.key_handler)
            self.panel.root.mainloop()
        

    def add_start_cells(self):
        for i in range(self.start_cells_num):
            self.grid.random_cell()
        
    def ai_handler(self):
        while True:
            if self.is_game_terminated():
                return
    
            self.grid.clear_flags()
            ai = Bot(self.grid.copy())
            move = ai.get_move()
            print("PERFORMING MOVE ", move, " ON GRID:")
            self.grid.print_grid()
            if (move in panel.DOWN_KEYS):
                self.grid.grid_down()
            elif (move in panel.RIGHT_KEYS):
                self.grid.grid_right()
            elif (move in panel.UP_KEYS):
                self.grid.grid_up()
            else:
                self.grid.grid_left()
                
            print('Score: {}'.format(self.grid.current_score))
            if self.grid.found_2048():
                self.you_win()
                if not self.keep_playing:
                    return

            if self.grid.moved:
                self.grid.random_cell()

            self.panel.paint()
            if not self.grid.can_move():
                self.over = True
                self.game_over()

            self.panel.root.update()  

    def key_handler(self, event):
        if self.is_game_terminated():
            return
        
        self.grid.clear_flags()

        key_value = event.keysym
        print('{} key pressed'.format(key_value))
        if key_value in GamePanel.UP_KEYS:
            self.grid.grid_up()
        elif key_value in GamePanel.LEFT_KEYS:
            self.grid.grid_left()
        elif key_value in GamePanel.DOWN_KEYS:
            self.grid.grid_down()
        elif key_value in GamePanel.RIGHT_KEYS:
            self.grid.grid_right()
        else:
            pass

        self.panel.paint()
        print('Score: {}'.format(self.grid.current_score))
        if self.grid.found_2048():
            self.you_win()
            if not self.keep_playing:
                return

        if self.grid.moved:
            self.grid.random_cell()

        self.panel.paint()
        if not self.grid.can_move():
            self.over = True
            self.game_over()

    def you_win(self):
        if not self.won:
            self.won = True
            print('You Win!')
            if messagebox.askyesno('2048', 'You Win!\n'
                                       'Are you going to continue the 2048 game?'):
                self.keep_playing = True

    def game_over(self):
        print('Game over!')
        messagebox.showinfo('2048', 'Oops!\n'
                                    'Game over!')


if __name__ == '__main__':
    size = 4
    grid = Grid(size)
    panel = GamePanel(grid)
    game2048 = Game(grid, panel, True)
    game2048.start()
