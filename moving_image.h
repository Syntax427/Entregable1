#ifndef MOVING_IMG_H
#define MOVING_IMG_H

#include <cstdio>
#include <iostream>
#include <stack>
#include <queue>
#include <string>
#include "basics.h"

enum ACTION {RIGHT, DOWN, LEFT, UP, REPEAT, ROTATE, UNDO, REDO};

struct Movement {
    ACTION action;
    int n;
    Movement(ACTION ac, int val) {
        this->action = ac;
        this->n = val;
    };
}; 

// Clase que representa una imagen como una colección de 3 matrices siguiendo el
// esquema de colores RGB

class moving_image {
private:
  unsigned char **red_layer; // Capa de tonalidades rojas
  unsigned char **green_layer; // Capa de tonalidades verdes
  unsigned char **blue_layer; // Capa de tonalidades azules
  std::queue<Movement> mainQueue;
  std::stack<Movement> lastActions;
  std::stack<Movement> lastUndo;
public:
  // Constructor de la imagen. Se crea una imagen por defecto
  moving_image() {
    // Reserva de memoria para las 3 matrices RGB
    red_layer = new unsigned char*[H_IMG];
    green_layer = new unsigned char*[H_IMG];
    blue_layer = new unsigned char*[H_IMG];
    
    for(int i=0; i < H_IMG; i++) {
      red_layer[i] = new unsigned char[W_IMG];
      green_layer[i] = new unsigned char[W_IMG];
      blue_layer[i] = new unsigned char[W_IMG];
    }

    // Llenamos la imagen con su color de fondo
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++) {
	red_layer[i][j] = DEFAULT_R;
	green_layer[i][j] = DEFAULT_G;
	blue_layer[i][j] = DEFAULT_B;
      }

    // Dibujamos el objeto en su posición inicial
    for(int i=0; i < 322; i++)
      for(int j=0; j < 256; j++) {
	if(!s_R[i][j] && !s_G[i][j] && !s_B[i][j]) {
	  red_layer[INIT_Y+i][INIT_X+j] = DEFAULT_R;
	  green_layer[INIT_Y+i][INIT_X+j] = DEFAULT_G;
	  blue_layer[INIT_Y+i][INIT_X+j] = DEFAULT_B;
	} else {
	  red_layer[INIT_Y+i][INIT_X+j] = s_R[i][j];
	  green_layer[INIT_Y+i][INIT_X+j] = s_G[i][j];
	  blue_layer[INIT_Y+i][INIT_X+j] = s_B[i][j];
	}
      }   
  }

  void printqueue() {
    for (int i = 0; i < this->mainQueue.size(); i++) {
        Movement actual = this->mainQueue.front();
        std::cout << actual.action << " " << actual.n << std::endl;
        this->mainQueue.pop();
        this->mainQueue.push(actual);
    }
  }

  // Destructor de la clase
  ~moving_image() {
    for(int i=0; i < H_IMG; i++) {
      delete red_layer[i];
      delete green_layer[i];
      delete blue_layer[i];
    }

    delete red_layer;
    delete green_layer;
    delete blue_layer;
  }

  // Función utilizada para guardar la imagen en formato .png
  void draw(const char* nb) {
    _draw(nb);
  }
  // Se hicieron las funciones privadas para decidir cuándo guardar en la queue y en el stack las acciones realizadas.
  void move_right(int d) {
    this->_move_right(d, true);
  }

  void move_down(int d) {
    this->_move_down(d, true);
  }

  void move_left(int d) {
    this->_move_left(d, true);
  }

  void move_up(int d) {
    this->_move_up(d, true);
  }

  void rotate() {
    this->_rotate(true);
  }
  // Undo que saca el movimiento más reciente (incluidos movimientos repetidos con repeat() y rehechos con redo()), hace la operación inversa y
  // mueve el movimiento al stack de undo, donde se guardan todos los movimientos de undo.
  void undo() {
    if (this->lastActions.empty()) return;
    this->mainQueue.push(Movement(UNDO, 0));
    Movement undidAction = this->lastActions.top();
    this->lastUndo.push(undidAction);
    this->lastActions.pop();
    switch (undidAction.action) {
        case RIGHT:
            this->_move_left(undidAction.n, false);
            break;
        case DOWN:
            this->_move_up(undidAction.n, false);
            break;
        case LEFT:
            this->_move_right(undidAction.n, false);
            break;
        case UP:
            this->_move_down(undidAction.n, false);
            break;
        case ROTATE:
            for (int i = 0; i < 3; i++) {
                this->_rotate(false);
            }
            break;
        default:
            break;
    }
  }
  // Comportamiento similar al undo, con la diferencia que saca el undo más reciente, lo realiza (ya que en el stack undo se guarda el movimiento que se deshizo)
  // y mueve de vuelta ese movimiento al stack de movimientos (ya que cuenta como una acción realizada).
  void redo() {
    if (this->lastUndo.empty()) return;
    this->mainQueue.push(Movement(REDO, 0));
    Movement redidAction = this->lastUndo.top();
    this->lastActions.push(redidAction);
    this->lastUndo.pop();
    switch (redidAction.action) {
        case RIGHT:
            this->_move_right(redidAction.n, false);
            break;
        case DOWN:
            this->_move_down(redidAction.n, false);
            break;
        case LEFT:
            this->_move_left(redidAction.n, false);
            break;
        case UP:
            this->_move_up(redidAction.n, false);
            break;
        case ROTATE:
            this->_rotate(false);
            break;
        default:
            break;
    }
  }

  // Función que obtiene el último movimiento realizado y lo repite, para luego guardarlo en el mismo stack de movimientos.
  void repeat() {
    if (this->lastActions.empty()) return;
    Movement lastAction = this->lastActions.top();
    this->lastActions.push(Movement(lastAction.action, lastAction.n));
    this->mainQueue.push(Movement(REPEAT, 0));
    switch (lastAction.action) {
        case RIGHT:
            this->_move_right(lastAction.n, false);
            break;
        case DOWN:
            this->_move_down(lastAction.n, false);
            break;
        case LEFT:
            this->_move_left(lastAction.n, false);
            break;
        case UP:
            this->_move_up(lastAction.n, false);
            break;
        case ROTATE:
            this->_rotate(false);
            break;
        default:
            break;
    }
  }

  // Crea una nueva imagen temporal y cicla por todo el queue de acciones, volviendo a insertar en la cola las acciones por las que pasa, mientras realiza y luego
  // genera una foto por cada una.
  void repeat_all() {
    moving_image* repeatImage = new moving_image();
    repeatImage->draw("repeat_mario_0.png");
    for (int i = 1; i <= this->mainQueue.size(); i++) {
        Movement actual = this->mainQueue.front();
        switch (actual.action) {
            case RIGHT:
                repeatImage->move_right(actual.n);
                break;
            case DOWN:
                repeatImage->move_down(actual.n);
                break;
            case LEFT:
                repeatImage->move_left(actual.n);
                break;
            case UP:
                repeatImage->move_up(actual.n);
                break;
            case ROTATE:
                repeatImage->rotate();
                break;
            case REPEAT:
                repeatImage->repeat();
                break;
            case UNDO:
                repeatImage->undo();
                break;
            case REDO:
                repeatImage->redo();
                break;
        }
        char name[20];
        std::sprintf(name, "repeat_mario_%d.png", i);
        repeatImage->draw(name);
        this->mainQueue.pop();
        this->mainQueue.push(actual);
    }
    delete repeatImage;
  }

private:
  // Función privada que guarda la imagen en formato .png
  void _draw(const char* nb) {
    //    unsigned char rgb[H_IMG * W_IMG * 3], *p = rgb;
    unsigned char *rgb = new unsigned char[H_IMG * W_IMG * 3];
    unsigned char *p = rgb;
    unsigned x, y;

    // La imagen resultante tendrá el nombre dado por la variable 'nb'
    FILE *fp = fopen(nb, "wb");

    // Transformamos las 3 matrices en una arreglo unidimensional
    for (y = 0; y < H_IMG; y++)
        for (x = 0; x < W_IMG; x++) {
            *p++ = red_layer[y][x];    /* R */
            *p++ = green_layer[y][x];    /* G */
            *p++ = blue_layer[y][x];    /* B */
        }
    // La función svpng() transforma las 3 matrices RGB en una imagen PNG 
    svpng(fp, W_IMG, H_IMG, rgb, 0);
    fclose(fp);
}
  // Función que similar desplazar la imagen, de manera circular, d pixeles a la izquierda
    void _move_left(int d, bool recordMovement) {
    // Agregar a la queue main y al stack de acciones
    if (recordMovement) {
        this->mainQueue.push(Movement(LEFT, d));
        this->lastActions.push(Movement(LEFT, d));
    }
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++) 
      tmp_layer[i] = new unsigned char[W_IMG];
    
    // Mover la capa roja
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
	      tmp_layer[i][j] = red_layer[i][j+d];      
    
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	  tmp_layer[i][j] = red_layer[i][k];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	  tmp_layer[i][j] = green_layer[i][j+d];      
    
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	  tmp_layer[i][j] = green_layer[i][k];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	  tmp_layer[i][j] = blue_layer[i][j+d];      
    
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	  tmp_layer[i][j] = blue_layer[i][k];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  blue_layer[i][j] = tmp_layer[i][j];
  }

  void _move_right(int d, bool recordMovement) {
    // Agregar a la queue main y al stack de acciones
    if (recordMovement) {
        this->mainQueue.push(Movement(RIGHT, d));
        this->lastActions.push(Movement(RIGHT, d));
    }
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++) 
      tmp_layer[i] = new unsigned char[W_IMG];
    
    // Mover la capa roja
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
	      tmp_layer[i][j+d] = red_layer[i][j];      
    
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	  tmp_layer[i][k] = red_layer[i][j];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	  tmp_layer[i][j+d] = green_layer[i][j];      
    
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	  tmp_layer[i][k] = green_layer[i][j];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	  tmp_layer[i][j+d] = blue_layer[i][j];      
    
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	  tmp_layer[i][k] = blue_layer[i][j];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  blue_layer[i][j] = tmp_layer[i][j];
  }

  void _move_up(int d, bool recordMovement) {
    // Agregar a la queue main y al stack de acciones
    if (recordMovement) {
        this->mainQueue.push(Movement(UP, d));
        this->lastActions.push(Movement(UP, d));
    }
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++) 
      tmp_layer[i] = new unsigned char[W_IMG];
    
    // Mover la capa roja
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
	      tmp_layer[i][j] = red_layer[i+d][j];      
    
    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[i][j] = red_layer[k][j];       

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[i][j] = green_layer[i+d][j];      
    
    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[i][j] = green_layer[k][j];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[i][j] = blue_layer[i+d][j];      
    
    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[i][j] = blue_layer[k][j];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  blue_layer[i][j] = tmp_layer[i][j];
  }

  void _move_down(int d, bool recordMovement) {
      // Agregar a la queue main y al stack de acciones
    if (recordMovement) {
        this->mainQueue.push(Movement(DOWN, d));
        this->lastActions.push(Movement(DOWN, d));
    }
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++) 
      tmp_layer[i] = new unsigned char[W_IMG];
    
    // Mover la capa roja
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
	      tmp_layer[i+d][j] = red_layer[i][j];      
    
    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[k][j] = red_layer[i][j];       

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[i+d][j] = green_layer[i][j];      
    
    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[k][j] = green_layer[i][j];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[i+d][j] = blue_layer[i][j];      
    
    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[k][j] = blue_layer[i][j];      

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  blue_layer[i][j] = tmp_layer[i][j];
  }

  void _rotate(bool recordMovement){
    // Agregar a la queue main y al stack de acciones
    if (recordMovement) {
        this->mainQueue.push(Movement(ROTATE, 0));
        this->lastActions.push(Movement(ROTATE, 0));
    }
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++) 
      tmp_layer[i] = new unsigned char[W_IMG];

    // Rotar la capa roja
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      tmp_layer[i][j] = red_layer[j][H_IMG-i-1];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  red_layer[i][j] = tmp_layer[i][j];

      // Rotar la capa verde
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      tmp_layer[i][j] = green_layer[j][H_IMG-i-1];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  green_layer[i][j] = tmp_layer[i][j];

      // Rotar la capa azul
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      tmp_layer[i][j] = blue_layer[j][H_IMG-i-1];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  blue_layer[i][j] = tmp_layer[i][j];    
  }

  
};

#endif
