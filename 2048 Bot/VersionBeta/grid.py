
import random

class Grid:
    '''The data structure representation of the 2048 game.
    '''
    def __init__(self, n):
        self.size = n
        self.cells = self.generate_empty_grid()
        self.compressed = False
        self.merged = False
        self.moved = False
        self.current_score = 0

    def copy(self):
        if not self.can_move():
            return False
        new_grid = Grid(self.size)
        new_grid.cells = self.cells
        new_grid.compressed = False
        new_grid.merged = False
        new_grid.moved = False
        new_grid.current_score = self.current_score
        return new_grid

    def can_move(self):
        return self.has_empty_cells() or self.can_merge()

    def random_cell(self):
        cell = random.choice(self.retrieve_empty_cells())
        i = cell[0]
        j = cell[1]
        self.cells[i][j] = 2 if random.random() < 0.9 else 4

    def retrieve_empty_cells(self):
        empty_cells = []
        for i in range(self.size):
            for j in range(self.size):
                if self.cells[i][j] == 0:
                    empty_cells.append((i, j))
        return empty_cells

    def generate_empty_grid(self):
        return [[0] * self.size for i in range(self.size)]

    def transpose(self):
        self.cells = [list(t) for t in zip(*self.cells)]

    def reverse(self):
        for i in range(self.size):
            start = 0
            end = self.size - 1
            while start < end:
                self.cells[i][start], self.cells[i][end] = \
                    self.cells[i][end], self.cells[i][start]
                start += 1
                end -= 1

    def clear_flags(self):
        self.compressed = False
        self.merged = False
        self.moved = False

    def left_compress(self):
        self.compressed = False
        new_grid = self.generate_empty_grid()
        for i in range(self.size):
            count = 0
            for j in range(self.size):
                if self.cells[i][j] != 0:
                    new_grid[i][count] = self.cells[i][j]
                    if count != j:
                        self.compressed = True
                    count += 1
        self.cells = new_grid

    def left_merge(self):
        self.merged = False
        for i in range(self.size):
            for j in range(self.size - 1):
                if self.cells[i][j] == self.cells[i][j + 1] and \
                   self.cells[i][j] != 0:
                    self.cells[i][j] *= 2
                    self.cells[i][j + 1] = 0
                    self.current_score += self.cells[i][j]
                    self.merged = True

    def found_2048(self):
        for i in range(self.size):
            for j in range(self.size):
                if self.cells[i][j] >= 2048:
                    return True
        return False

    def has_empty_cells(self):
        for i in range(self.size):
            for j in range(self.size):
                if self.cells[i][j] == 0:
                    return True
        return False

    def can_merge(self):
        for i in range(self.size):
            for j in range(self.size - 1):
                if self.cells[i][j] == self.cells[i][j + 1]:
                    return True
        for j in range(self.size):
            for i in range(self.size - 1):
                if self.cells[i][j] == self.cells[i + 1][j]:
                    return True
        return False

    def set_cells(self, cells):
        self.cells = cells

    def print_grid(self):
        print('-' * 40)
        for i in range(self.size):
            for j in range(self.size):
                print('%d\t' % self.cells[i][j], end='')
            print()
        print('-' * 40)

    def grid_up(self):
        #print("CURRENT GRID FOR GRID_UP COPY: ")
        #self.print_grid()
        #print("MOVED UP!!!")
        self.transpose()
        #print("GRID AFTER TRANSPOSE: ")
        #self.print_grid()

        self.left_compress()
        #print("GRID AFTER LEFT COMPRESS: ")
        #self.print_grid()

        self.left_merge()
        #print("GRID AFTER LEFT MERGE: ")
        #self.print_grid()

        self.moved = self.compressed or self.merged
        #print("MOVED?: ", self.moved)

        self.left_compress()
        #print("GRID AFTER LEFT COMPRESS: ")
        #self.print_grid()

        self.transpose()
        #print("GRID AFTER TRANSPOSE BACK TO : ")
        #self.print_grid()

    def grid_left(self):
        #print(self, "BEING MODIFIED: GRID_LEFT")
        self.left_compress()
        self.left_merge()
        self.moved = self.compressed or self.merged
        self.left_compress()

    def grid_down(self):
        #print(self, "BEING MODIFIED: GRID_DOWN")
        self.transpose()
        self.reverse()
        self.left_compress()
        self.left_merge()
        self.moved = self.compressed or self.merged
        self.left_compress()
        self.reverse()
        self.transpose()

    def grid_right(self):
        #print(self, "BEING MODIFIED: GRID_RIGHTx")
        #print("CURRENT GRID FOR GRID_RIGHT COPY: ")
        #self.print_grid()

        self.reverse()
        #print("GRID_RIGHT AFTER REVERSE: ")
        #self.print_grid()

        self.left_compress()
        #print("GRID_RIGHT AFTER LEFT COMPRESS: ")
        #self.print_grid()

        self.left_merge()
        #print("GRID_RIGHT AFTER LEFT MERGE: ")
        #self.print_grid()

        self.moved = self.compressed or self.merged

        self.left_compress()
        #print("GRID_RIGHT AFTER LEFT COMPRESS: ")
        #self.print_grid()

        self.reverse()
        #print("GRID RIGHT AFTER REVERSE TO NORMAL: ")
        #self.print_grid()