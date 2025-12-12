import sys
import matplotlib.pyplot as plt

def load_binary_file(filename):
    with open(filename, 'r') as f:
        values = [int(line.strip()) for line in f if line.strip() in ('0', '1')]
    
    first_zero = next((i for i, v in enumerate(values) if v == 0), 0)
    last_zero = next((i for i, v in enumerate(reversed(values)) if v == 0), 0)
    return values[first_zero:len(values) - last_zero]

def plot_binary_files(filenames):
    fig, axes = plt.subplots(len(filenames), 1, figsize=(12, 3 * len(filenames)), sharex=True)
    
    if len(filenames) == 1:
        axes = [axes]
    
    for ax, filename in zip(axes, filenames):
        values = load_binary_file(filename)
        ax.step(range(len(values)), values, where='post')
        ax.set_ylim(-0.1, 1.1)
        ax.set_ylabel('Value')
        ax.set_title(filename)
    
    axes[-1].set_xlabel('Index')
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_binary_files(sys.argv[1:] if len(sys.argv) > 1 else ["data.txt"])
