import time

# Mock TigerEngine for testing
class MockTigerEngine:
    def __init__(self, responses):
        self.responses = responses
        self.current_response = 0

    def readline(self):
        if self.current_response < len(self.responses):
            response = self.responses[self.current_response]
            self.current_response += 1
            return response
        return ""

# Extract moved piece from TigerEngine's response
def extract_moved_piece(piece_info):
    return piece_info.strip()

# Communicate with Arduino
def communicate_with_arduino(command):
    print(f"Command sent to Arduino: {command}")
    return "OK"

# Piece type mappings
piece_types = {
    "K": "King",
    "Q": "Queen",
    "R": "Rook",
    "B": "Bishop",
    "N": "Knight",
    "P": "Pawn",
    # Add more mappings if needed
}

def test_tiger_move(responses):
    tigerengine = MockTigerEngine(responses)
    moves = []
    start_time = time.time()
    timeout = 5  # seconds

    while time.time() - start_time < timeout:
        tigerMove = tigerengine.readline().strip()
        if tigerMove:
            print(f"TigerEngine's move: {tigerMove}")
            try:
                cleanedUpTigerMoveResponse = tigerMove.split("bestmove ")[1]  # removes the 'bestmove' part of the response
                splitResponse = cleanedUpTigerMoveResponse.split("|")  # splits the response by the "|" delimiter
                moveString = splitResponse[0]
                movedPiece = extract_moved_piece(splitResponse[1])
                movedPiece = piece_types.get(movedPiece, "Unknown Piece")

                # Add move response from engine to move history
                moves.append(moveString)

                if "capturedPiece" in cleanedUpTigerMoveResponse:
                    capturedPiece = extract_moved_piece(splitResponse[2])
                    capturedPiece = piece_types.get(capturedPiece, "Unknown Piece")
                    arduinoCommandString = f"docapturemove {moveString} {movedPiece} {capturedPiece}"
                    arduino_response = communicate_with_arduino(arduinoCommandString)
                    print(f"Response from Arduino: {arduino_response}")
                else:
                    # Make robot perform normal quiet move
                    arduinoCommandString = f"doquietmove {moveString} {movedPiece}"
                    arduino_response = communicate_with_arduino(arduinoCommandString)
                    print(f"Response from Arduino: {arduino_response}")
                break
            except IndexError:
                print("Malformed TigerEngine response.")
                break
        time.sleep(0.1)  # Simulate waiting for a response
    else:
        print("Timeout waiting for TigerEngine's move.")

# Example usage
if __name__ == "__main__":
    # Define test TigerEngine responses
    test_responses = [
        "bestmove d8d5|movedPiece:4|capturedPiece:0",  # Simulating a capture
    ]
    
    for response in test_responses:
        print("\nTesting response:", response)
        test_tiger_move([response])
