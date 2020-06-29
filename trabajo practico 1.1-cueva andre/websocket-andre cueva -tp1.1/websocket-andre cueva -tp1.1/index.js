var express = require('express');
var socket = require('socket.io');

// App setup
var app = express();
var server = app.listen(4000, function(){
    console.log('listening for requests on port 4000,');
});

// Static files
app.use(express.static('public'));

// Socket setup & pass server
var io = socket(server);
io.on('connection', (socket) => {

	console.log('------------------nueva conexion----------------------');
	console.log('made socket connection', socket.id);

	socket.on('clickOfAdmin', function(){
         console.log('Nuevo administrador');
         socket.emit('clickOfAdmin');
	});

	socket.on('clickOfUser', function(Name){ //llega el evento chat al servidor
        console.log('Se ha conectado ',Name);
        socket.broadcast.emit('clickOfUser', Name, socket.id);//envio a todos y a mi mismo
	});

	socket.on('disconnect', function(){//debe usarse disconnect para que cuando cierras la pestana
                                       //aparesca el mensaje por consola y cambie el estado del html
            console.log('usuario DESCONECTADOOOOOOO!!!!!!!!!!');
            socket.broadcast.emit('disconnectOfUser',socket.id);//envio a todos y a mi mismo
    });


});