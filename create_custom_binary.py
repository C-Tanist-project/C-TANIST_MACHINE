
import struct

def create_custom_binary():
    values = []
    values.extend([0] * 32)
    # sequence = [130, 1, 142, 2, 14, 41, 46, 41, 11, 42, 3] mult test
    # sequence = [130, 50, 134, 10, 6, 41, 38, 41, 11, 42, 18] sub test
    # sequence = [130, -10, 5, 37, 0, 38, 130, 10, 4, 42, 44, 0, 130, 10, 1, 48, 52, 0, 0, 0, 0, 0, 130, 15, 11]
    # sequence = [32, 40, 0, 0, 0, 0, 0, 0, 41, 50] # br ind
    # sequence = [0, 40, 0, 0, 0, 0, 0, 0, 50] # br dir
    # sequence = [130, 15, 7, 43, 3, 45, 39, 43, 34, 43, 11, 44, 56, 7] store, add
    # sequence = [131, 15, 3, 41, 35, 41, 11, 0, 0, 42, 45] load
    # sequence = [35, 41, 130, 11, 7, 12, 0, 20, 11, 25]
    values.extend(sequence)
    
    with open('custom_program.bin', 'wb') as f:
        for value in values:
            f.write(struct.pack('<h', value))
    
    print(f"Arquivo 'custom_program.bin' criado com {len(values)} valores:")
    print(f"- Posições 0-32: zeros")
    print(f"- Posições 33-41: {sequence}")
    print("\nConteúdo completo:")
    for i, val in enumerate(values):
        if i < 10 or i >= 33:  # Mostra os primeiros 10 e a sequência
            print(f"  [{i:2d}]: {val}")
        elif i == 10:
            print("  ...  (zeros até posição 32)")

if __name__ == "__main__":
    create_custom_binary()
