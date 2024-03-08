// additional copyright/license info:
//© All Rights Reserved
//
//Chess.com Super Monkey Vision by CrazyUfo
//
// ==UserScript==
// @name         Chess monkey vision
// @namespace    CrazyUfo
// @version      1.0.0
// @description  Chess.com Super Vision
// @author       CrazyUfo
// @license      Chess.com Bot/Cheat © 2023 by MrAuzzie#998142, © All Rights Reserved
// @match        https://www.chess.com/play/*
// @match        https://www.chess.com/game/*
// @icon         data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==
// @require      https://code.jquery.com/jquery-3.6.0.min.js
// @run-at       document-start
// ==/UserScript==


function buildDebugField() {
    let debugDiv = document.createElement("div");

    // Apply styles to the div
    debugDiv.style.position = "fixed"; // Use 'fixed' to stick it at the bottom regardless of scrolling
    debugDiv.style.left = "30%"; // Center horizontally
    debugDiv.style.transform = "translateX(-50%)"; // Adjust for centering
    debugDiv.style.bottom = "0"; // Stick to the bottom
    debugDiv.style.width = "500px";
    debugDiv.style.height = "350px";
    debugDiv.style.backgroundColor = "#F1F1F1"; // Just an example, change as needed
    debugDiv.style.fontFamily = "monospace"; // Set font to monospace
    // Ensure text can be selected. This line is usually not necessary unless user-select was previously set to none.
    debugDiv.style.userSelect = "auto";
    // Append the div to the body
    document.body.appendChild(debugDiv);

    return debugDiv;
}

function genNumbers(attackBoards) {
    let boardSvg = document.querySelector('.coordinates');

    let originalBoard = `
        <text x="0.75" y="3.5" font-size="2.8" className="coordinate-light">8</text>
        <text x="0.75" y="15.75" font-size="2.8" className="coordinate-dark">7</text>
        <text x="0.75" y="28.25" font-size="2.8" className="coordinate-light">6</text>
        <text x="0.75" y="40.75" font-size="2.8" className="coordinate-dark">5</text>
        <text x="0.75" y="53.25" font-size="2.8" className="coordinate-light">4</text>
        <text x="0.75" y="65.75" font-size="2.8" className="coordinate-dark">3</text>
        <text x="0.75" y="78.25" font-size="2.8" className="coordinate-light">2</text>
        <text x="0.75" y="90.75" font-size="2.8" className="coordinate-dark">1</text>
        <text x="10" y="99" font-size="2.8" className="coordinate-dark">a</text>
        <text x="22.5" y="99" font-size="2.8" className="coordinate-light">b</text>
        <text x="35" y="99" font-size="2.8" className="coordinate-dark">c</text>
        <text x="47.5" y="99" font-size="2.8" className="coordinate-light">d</text>
        <text x="60" y="99" font-size="2.8" className="coordinate-dark">e</text>
        <text x="72.5" y="99" font-size="2.8" className="coordinate-light">f</text>
        <text x="85" y="99" font-size="2.8" className="coordinate-dark">g</text>
        <text x="97.5" y="99" font-size="2.8" className="coordinate-light">h</text>
    `;

    const squareSize = 12.5;

    function renderBoard(attackBoard, boardSvg) {
        for (let row = 0; row < attackBoard.length; row++) {
            for (let col = 0; col < attackBoard[row].length; col++) {
                // Use '.' to represent an empty square for better readability
                const content = attackBoard[row][col] || '';
                const x=10 + squareSize*col;
                const y=91 - squareSize*row;
                boardSvg.innerHTML += `<text x="${x}" y="${y}" font-size="2.8" className="coordinate-dark">${content}</text>`;
            }
        }
    }

    boardSvg.innerHTML = originalBoard;
    boardSvg.innerHTML += `<text x="22.5" y="53.25" font-size="2.8" className="coordinate-dark">1</text>`;
    boardSvg.innerHTML += `<text x="10" y="3.5" font-size="2.8" className="coordinate-dark">w</text>`;

    let whiteBoard = attackBoards[0];
    let blackBoard = attackBoards[1];
    renderBoard(whiteBoard, boardSvg);

}

function createBoardMap(boardDivs) {
    // Initialize an 8x8 board with null (empty squares)
    let boardMap = Array(8).fill(null).map(() => Array(8).fill(null));

    boardDivs.forEach(div => {
        let pieceType = '';
        let squareClass = '';

        Array.from(div.classList).forEach(className => {
            if (className.startsWith('square-')) {
                squareClass = className;
            } else if (className !== 'piece') {
                pieceType = className;
            }
        });

        const pos = squareClass.replace("square-", ""); // e.g., '82'

        // Convert to 0-based index
        let col = parseInt(pos[0], 10) - 1;
        let row = parseInt(pos[1], 10) - 1;


        boardMap[row][col] = pieceType;
    });

    return boardMap;
}

function renderBoard(boardMap, debugDiv) {
    let boardText = '';
    for (let i = 0; i < boardMap.length; i++) {
        for (let j = 0; j < boardMap[i].length; j++) {
            // Use '.' to represent an empty square for better readability
            const squareContent = boardMap[i][j] || '.';
            boardText += squareContent.padEnd(3, ' '); // Pad for alignment
        }
        boardText += '<br>'; // New line for each row
    }
    debugDiv.innerHTML += boardText;
}

function processBoard(pieces, debugDiv) {
    for (let i = 0; i < pieces.length; i++) {
        let piece = pieces[i];

        // Assuming the class name is in the format "piece [type] square-[position]"
        const pieceType = piece.classList[1]; // This gets the piece type (e.g., "br")
        let squareName = piece.classList[2];
        let posStr = squareName.replace("square-", ""); // Removes 'square-' prefix, resulting in "88"

        // Extract the digits and convert them to zero-based indexes
        let col = parseInt(posStr[0], 10) - 1; // Convert to zero-based index
        let row = parseInt(posStr[1], 10) - 1; // Convert to zero-based index

        // Initialize an empty attack map
        let attackMap = Array(8).fill().map(() => Array(8).fill(0));

        // Calculate attack maps based on piece type
        if (pieceType === 'br' || pieceType === 'wr') { // Example for Rooks
            attackMap = generateAttackMapForRook(row, col);
        }
    }
}

function main() {

    let debugDiv = buildDebugField();
    // Optional: Add some content to the div
    debugDiv.innerHTML = "This is a sticky div at the bottom of the page.";

    (function superVisionLoop() {
        console.log("tick");

        // Always update boardDivs to catch new or removed pieces
        let boardDivs = document.querySelectorAll('.piece');

        if (boardDivs.length !== 0) {
            debugDiv.innerHTML = `Loaded pieces DIVs len ${boardDivs.length}`;
            //processBoard(boardDivs, debugDiv);
            let board = createBoardMap(boardDivs);
            genNumbers([board,[]]);
            renderBoard(board, debugDiv);
        }

        // Restart this function after timeout
        setTimeout(superVisionLoop, 1000);
    })();
}

window.addEventListener("load", (event) => {
    main();
});
