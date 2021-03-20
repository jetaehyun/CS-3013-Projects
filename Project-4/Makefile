all: circusJazz shipShapingUp

circusJazz: circusJazz.c
		gcc -Wall circusJazz.c -o circusJazz -lpthread

shipShapingUp: shipShapingUp.c linked_list.c
		gcc -Wall shipShapingUp.c linked_list.c -o shipShapingUp -lpthread -lrt

clean:
		rm -f circusJazz shipShapingUp
