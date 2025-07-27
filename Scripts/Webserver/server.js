/**
 * An all-in-one Node.js server that:
 * 1. Receives POST requests from an Arduino/SIM800L on the '/data' endpoint.
 * 2. Overwrites the device's timestamp with the server's current time.
 * 3. Saves the data to a MySQL database.
 * 4. Serves an HTML page at '/' that displays all the data from the MySQL database.
 * 5. Serves the required CSS file at '/style.css'.
 * 6. Handles DELETE requests to remove a single data point.
 * 7. Handles a DELETE request to remove ALL data points from the database.
 *
 * HOW TO RUN:
 * 1. Install required packages: npm install mysql2
 * 2. Ensure you have `index.html` and `style.css` files in the same directory.
 * 3. UPDATE THE `dbConfig` OBJECT BELOW with your MySQL server details.
 * 4. UPDATE THE `TABLE_NAME` constant with the name of your table.
 * 5. Run the server: node server.js
 */

const http = require('http');
const fs = require('fs');
const path = require('path');
const mysql = require('mysql2/promise');

// --- CONFIGURATION ---
const dbConfig = {
    host: 'localhost',
    user: 'root',
    password: 'bestfriends1.',
    database: 'esp32_data'
};
const TABLE_NAME = 'drone_data'; // <-- IMPORTANT: Change this to your table name
const PORT = 25565;
// --- END CONFIGURATION ---

// Create the HTTP server.
const server = http.createServer((req, res) => {
    if (req.url === '/' && req.method === 'GET') {
        serveHtml(res);
    }
    else if (req.url === '/style.css' && req.method === 'GET') {
        serveCss(res);
    }
    else if (req.url === '/api/data' && req.method === 'GET') {
        serveDataApi(res);
    }
    else if (req.url === '/data' && req.method === 'POST') {
        receiveArduinoData(req, res);
    }
    else if (req.url === '/api/data/all' && req.method === 'DELETE') {
        deleteAllData(res);
    }
    else if (req.url.match(/\/api\/data\/(\d+)/) && req.method === 'DELETE') {
        const id = parseInt(req.url.split('/')[3]);
        deleteDataPoint(res, id);
    }
    else {
        res.writeHead(404, { 'Content-Type': 'text/plain' });
        res.end('Not Found');
    }
});

server.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
    console.log(`Listening for Arduino POST requests on /data`);
});

/**
 * Handles incoming POST requests from the Arduino.
 */
async function receiveArduinoData(req, res) {
    let body = '';
    req.on('data', chunk => { body += chunk.toString(); });
    req.on('end', async () => {
        console.log('---------------------------');
        console.log('Received POST request from Arduino on /data');
        console.log('Request Body:', body);
        let data;
        try { data = JSON.parse(body); } 
        catch (error) {
            console.error('Error parsing JSON from Arduino:', error);
            res.writeHead(400, { 'Content-Type': 'text/plain' });
            res.end('Invalid JSON format.');
            return;
        }
        const now = new Date();
        data.timestamp = now.toISOString().slice(0, 19).replace('T', ' ');
        console.log('Overwrote timestamp with server time:', data.timestamp);
        const columns = Object.keys(data).map(key => `\`${key}\``).join(', ');
        const placeholders = Object.keys(data).map(() => '?').join(', ');
        const values = Object.values(data);
        const sql = `INSERT INTO ${TABLE_NAME} (${columns}) VALUES (${placeholders})`;
        console.log('Executing SQL:', sql);
        console.log('With Values:', values);
        let connection;
        try {
            connection = await mysql.createConnection(dbConfig);
            await connection.execute(sql, values);
            console.log('Successfully inserted data into MySQL.');
            res.writeHead(201, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ message: 'Data received and saved successfully.'}));
        } catch (dbError) {
            console.error('Database Error:', dbError);
            res.writeHead(500, { 'Content-Type': 'text/plain' });
            res.end('Error saving data to the database.');
        } finally {
            if (connection) await connection.end();
        }
        console.log('---------------------------\n');
    });
}

/**
 * Serves all data from the database to the front-end webpage.
 */
async function serveDataApi(res) {
    let connection;
    try {
        connection = await mysql.createConnection(dbConfig);
        const [results] = await connection.execute(`SELECT * FROM ${TABLE_NAME} ORDER BY timestamp DESC`);
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify(results));
    } catch (err) {
        console.error('API Error:', err);
        res.writeHead(500, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: 'Database operation failed' }));
    } finally {
        if (connection) await connection.end();
    }
}

/**
 * Deletes a single data point from the database by its ID.
 */
async function deleteDataPoint(res, id) {
    console.log(`Received DELETE request for id: ${id}`);
    if (isNaN(id)) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: 'Invalid ID supplied' }));
        return;
    }
    let connection;
    try {
        connection = await mysql.createConnection(dbConfig);
        const sql = `DELETE FROM ${TABLE_NAME} WHERE id = ?`;
        const [result] = await connection.execute(sql, [id]);
        if (result.affectedRows > 0) {
            console.log(`Successfully deleted record with id: ${id}`);
            res.writeHead(200, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ message: 'Data point deleted successfully' }));
        } else {
            console.log(`No record found with id: ${id}`);
            res.writeHead(404, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: 'Data point not found' }));
        }
    } catch (dbError) {
        console.error('Database Error during deletion:', dbError);
        res.writeHead(500, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: 'Error deleting data from the database' }));
    } finally {
        if (connection) await connection.end();
    }
}

/**
 * Deletes ALL data from the database table.
 */
async function deleteAllData(res) {
    console.log(`Received request to DELETE ALL DATA`);
    let connection;
    try {
        connection = await mysql.createConnection(dbConfig);
        const sql = `TRUNCATE TABLE ${TABLE_NAME}`;
        await connection.execute(sql);
        console.log(`Successfully truncated table: ${TABLE_NAME}`);
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ message: 'All data deleted successfully' }));
    } catch (dbError) {
        console.error('Database Error during TRUNCATE:', dbError);
        res.writeHead(500, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: 'Error deleting all data from the database' }));
    } finally {
        if (connection) await connection.end();
    }
}


/** Serves the index.html file. */
function serveHtml(res) {
    const htmlFilePath = path.join(__dirname, 'index.html');
    fs.readFile(htmlFilePath, 'utf8', (err, data) => {
        if (err) {
            console.error('Error reading index.html:', err);
            res.writeHead(500, { 'Content-Type': 'text/plain' });
            res.end('Error loading HTML file.');
            return;
        }
        res.writeHead(200, { 'Content-Type': 'text/html' });
        res.end(data);
    });
}

/** Serves the style.css file. */
function serveCss(res) {
    const cssFilePath = path.join(__dirname, 'style.css');
    fs.readFile(cssFilePath, 'utf8', (err, data) => {
        if (err) {
            console.error('Error reading style.css:', err);
            res.writeHead(500, { 'Content-Type': 'text/plain' });
            res.end('Error loading CSS file.');
            return;
        }
        res.writeHead(200, { 'Content-Type': 'text/css' });
        res.end(data);
    });
}
