require('dotenv').config();

const fs = require('fs');
const path = require('path');
const axios = require('axios'); // npm install dotenv axios

const apiKey = process.env.ELEVENLABS_API_KEY;
const voiceId = 'kcQkGnn0HAT2JRDQ4Ljp';

const figuras = [
    //{ pista: '0001', texto: '¡Cuadrado!' },
    //{ pista: '0002', texto: '¡Círculo!' },
    //{ pista: '0003', texto: '¡Trapecio!' },
    //{ pista: '0004', texto: '¡Hexágono!' },
    //{ pista: '0005', texto: '¡Triángulo!' },
    //{ pista: '0006', texto: '¡Rectángulo!' },
    //{ pista: '0007', texto: '¡Rombo!' },
    //{ pista: '0008', texto: '¡Pentágono!' }
    //{ pista: '0009', texto: '¡Hola! Coloca una figura en el tablero para comenzar.' } // Audio de bienvenida
    { pista: '0010', texto: '¡Un Entero!' },
    { pista: '0011', texto: '¡Un Medio!' },
    { pista: '0012', texto: '¡Un Tercio!' },
    { pista: '0013', texto: '¡Un Cuarto!' },
    { pista: '0014', texto: '¡Un Sexto!' },
    { pista: '0015', texto: '¡Un Octavo!' }
];

const outputDir = path.join(__dirname, '../Assets/audio_sd');
if (!fs.existsSync(outputDir)) {
    fs.mkdirSync(outputDir, { recursive: true });
}

async function generarVoz(figura) {
    const url = `https://api.elevenlabs.io/v1/text-to-speech/${voiceId}`;

    const headers = {
        'Accept': 'audio/mpeg',
        'xi-api-key': apiKey,
        'Content-Type': 'application/json'
    };

    const data = {
        text: figura.texto,
        model_id: 'eleven_multilingual_v2',
        voice_settings: {
            stability: 0.5,
            similarity_boost: 0.75
        }
    };

    try {
        console.log(`Generando audio para: ${figura.texto}...`);
        const response = await axios.post(url, data, { headers, responseType: 'stream' });

        const filePath = path.join(outputDir, `${figura.pista}.mp3`);
        const writer = fs.createWriteStream(filePath);
        response.data.pipe(writer);

        return new Promise((resolve, reject) => {
            writer.on('finish', () => {
                console.log(`✅ Guardado: ${figura.pista}.mp3`);
                resolve();
            });
            writer.on('error', reject);
        });
    } catch (error) {
        const status = error.response ? error.response.status : 'sin respuesta';
        console.error(`❌ Error con "${figura.texto}" (HTTP ${status}):`, error.message);
    }
}

async function ejecutar() {
    if (!apiKey) {
        console.error('Falta ELEVENLABS_API_KEY en tu archivo .env');
        return;
    }
    for (const fig of figuras) {
        await generarVoz(fig);
    }
    console.log('¡Todos los audios generados!');
}

ejecutar();