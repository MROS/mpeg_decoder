const fs = require("fs");

const DIR = "./huffman_tables";

const files = fs.readdirSync(DIR);

function analyze_code (codes) {
    codes.sort((a, b) => {
        if (a.length == b.length) {
            return parseInt(a) - parseInt(b);
        } else {
            return a.length - b.length;
        }
    });
    for (let i = 1; i < codes.length; i++) {
        if (codes[i].length == codes[i - 1].length) {
            if (parseInt(codes[i], 2) - parseInt(codes[i - 1], 2) != 1) {
                console.log(`${codes[i - 1]} ${codes[i]} 相差不爲 1`);
            }
        }
    }
}

for (let file of files) {
    console.log(`解析 ${file}`);
    const content = fs.readFileSync(`${DIR}/${file}`).toString();
    const lines = content.split("\n").filter(l => l.length > 0);
    const codes = lines.map(l => l.split(" ")[0]);
    analyze_code(codes);
}