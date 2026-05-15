# Functions for generating the plots
from typing import Dict
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from utils import *
from constants import *
import scipy.stats as stats
import matplotlib.lines as mlines
import matplotlib.ticker as mticker
from pathlib import Path
from itertools import product


##########################################################
################ GRAFICAS DE DEBUG RANGES ################
##########################################################
import matplotlib.ticker as mticker

def plot_pruning_points(csv_path: str, save: bool = False) -> None:
    df = pd.read_csv(csv_path)
    dataset_name = Path(csv_path).stem

    # Detectar configuración
    has_threshold = 'threshold' in df.columns
    has_max_leaf  = 'maxPointsLeaf' in df.columns

    def config_label(row):
        parts = []
        if has_threshold: parts.append(f'thr={int(row["threshold"])}')
        if has_max_leaf:  parts.append(f'maxL={int(row["maxPointsLeaf"])}')
        return ' | '.join(parts) if parts else 'default'

    df['config'] = df.apply(config_label, axis=1)
    configs = df['config'].unique()

    # Agregamos vecinos (neighbours) a la agrupación
    best_all = df.loc[df.groupby(
        ['leaf', 'kernel', 'mode', 'radius', 'config'])['count'].idxmin()
    ].copy()

    agg = (
        best_all.groupby(['mode', 'kernel', 'radius', 'config'])
        .agg(
            total_pts=('total', 'sum'), 
            evaluated_pts=('count', 'sum'),
            neighbor_pts=('neighbours', 'sum') # Nueva columna de vecinos reales
        )
        .reset_index()
    )
    
    # Porcentajes solicitados
    agg['pct_evaluated'] = (agg['evaluated_pts'] / agg['total_pts']) * 100
    agg['pct_neighbors_total'] = (agg['neighbor_pts'] / agg['total_pts']) * 100

    modes   = agg['mode'].unique()
    kernels = sorted(agg['kernel'].unique())
    radii   = sorted(agg['radius'].unique())

    for mode in modes:
        for config in configs:
            df_mode = agg[(agg['mode'] == mode) & (agg['config'] == config)]
            best    = best_all[(best_all['mode'] == mode) & (best_all['config'] == config)]

            if df_mode.empty: continue

            n_kernels = len(kernels)
            n_radii   = len(radii)
            
            # Ajuste de anchos para 3 barras (Total, Evaluated, Neighbors)
            group_width = 0.85
            bar_width   = group_width / (n_radii * 3.5) 

            x = np.arange(n_kernels)
            colors_total = plt.cm.Blues(np.linspace(0.4, 0.7, n_radii))
            colors_eval  = plt.cm.Oranges(np.linspace(0.4, 0.7, n_radii))
            colors_neigh = plt.cm.Greens(np.linspace(0.4, 0.7, n_radii))

            fig, ax = plt.subplots(figsize=(max(12, n_kernels * 3), 7))

            for ri, radius in enumerate(radii):
                df_r = df_mode[df_mode['radius'] == radius].set_index('kernel')

                # Centro del grupo para este radio dentro del kernel
                # Desplazamiento para el trío de barras
                base_offset = (ri - (n_radii - 1) / 2) * (bar_width * 3.8)
                
                offset_total = base_offset - bar_width
                offset_eval  = base_offset
                offset_neigh = base_offset + bar_width

                totals    = [df_r.loc[k, 'total_pts']     if k in df_r.index else 0 for k in kernels]
                evaluated = [df_r.loc[k, 'evaluated_pts'] if k in df_r.index else 0 for k in kernels]
                neighbors = [df_r.loc[k, 'neighbor_pts']  if k in df_r.index else 0 for k in kernels]
                
                pct_eval  = [df_r.loc[k, 'pct_evaluated']      if k in df_r.index else 0 for k in kernels]
                pct_neigh = [df_r.loc[k, 'pct_neighbors_total'] if k in df_r.index else 0 for k in kernels]

                # Dibujo de barras
                ax.bar(x + offset_total, totals, bar_width, label=f'r={radius} Total',
                       color=colors_total[ri], edgecolor='black', linewidth=0.3)
                
                bars_ev = ax.bar(x + offset_eval, evaluated, bar_width, label=f'r={radius} Evaluados',
                                 color=colors_eval[ri], edgecolor='black', linewidth=0.3)
                
                bars_ne = ax.bar(x + offset_neigh, neighbors, bar_width, label=f'r={radius} Vecinos',
                                 color=colors_neigh[ri], edgecolor='black', linewidth=0.3)

                # Etiquetas de porcentaje arriba de barras
                for bi, bar in enumerate(bars_ev):
                    if pct_eval[bi] > 0:
                        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.1,
                                f'{pct_eval[bi]:.1f}%', ha='center', va='bottom', 
                                fontsize=7, rotation=90)
                
                for bi, bar in enumerate(bars_ne):
                    if pct_neigh[bi] > 0:
                        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.1,
                                f'{pct_neigh[bi]:.1f}%', ha='center', va='bottom', 
                                fontsize=7, color='darkgreen', rotation=90)

            ax.set_xticks(x)
            ax.set_xticklabels([k.upper() for k in kernels], fontsize=11)
            
            ax.set_ylabel('Nº de Puntos (Escala Log)', fontsize=12)
            ax.yaxis.set_major_formatter(mticker.FuncFormatter(
                lambda val, _: f'{val/1e6:.1f}M' if val >= 1e6 else f'{val/1e3:.0f}K' if val >= 1e3 else f'{val:.0f}'))
            
            ax.set_title(f'Puntos: Total vs Evaluados (Poda) vs Vecinos Reales\n'
                         f'Dataset: {dataset_name} | Modo: {mode} | {config}', fontsize=12, fontweight='bold')
            
            # Leyenda fuera para no tapar barras
            ax.legend(loc='upper left', bbox_to_anchor=(1, 1), fontsize=8, ncol=1)
            ax.grid(axis='y', which='both', alpha=0.2)
            
            plt.tight_layout()
            plt.show() # Muestra la gráfica

            if save:
                safe_config = config.replace(' | ', '_').replace('=', '')
                out_path = Path(csv_path).parent / f'{dataset_name}_{mode}_{safe_config}_efficiency.png'
                fig.savefig(out_path, dpi=150, bbox_inches='tight')
                print(f'Guardado: {out_path}')
            
            plt.close(fig) # SOLUCIÓN: Cierra explícitamente para limpiar memoria

def plot_pruning_distribution(csv_path: str, save: bool = False) -> None:
    df = pd.read_csv(csv_path)
    dataset_name = Path(csv_path).stem

    # Detectar si hay columnas de configuración
    has_threshold = 'threshold' in df.columns
    has_max_leaf  = 'maxPointsLeaf' in df.columns

    # Construir etiqueta de configuración por fila
    def config_label(row):
        parts = []
        if has_threshold:
            parts.append(f'thr={int(row["threshold"])}')
        if has_max_leaf:
            parts.append(f'maxL={int(row["maxPointsLeaf"])}')
        return ' | '.join(parts) if parts else 'default'

    df['config'] = df.apply(config_label, axis=1)
    configs = df['config'].unique()

    best = df.loc[df.groupby(['leaf', 'kernel', 'mode', 'radius'])['count'].idxmin()].copy()
    best['pct_pruned'] = (1.0 - best['count'] / best['total']) * 100.0


    # Hojas totales por kernel (antes de filtrar pct=0)
    total_leaves_by_kernel = (
        best.groupby(['mode', 'kernel', 'radius', 'config'])['leaf']
        .nunique()
        .reset_index()
        .rename(columns={'leaf': 'total_leaves'})
    )

    pruned = best[best['pct_pruned'] > 0.01]

    modes   = pruned['mode'].unique()
    kernels = sorted(pruned['kernel'].unique())
    radii   = sorted(pruned['radius'].unique())

    for mode in modes:
        df_mode  = pruned[pruned['mode'] == mode]
        df_total = total_leaves_by_kernel[total_leaves_by_kernel['mode'] == mode]

        n_kernels = len(kernels)
        fig, axes = plt.subplots(1, n_kernels,
                                  figsize=(7 * n_kernels, 8),
                                  sharey=True)
        if n_kernels == 1:
            axes = [axes]

        colors = plt.cm.viridis(np.linspace(0.2, 0.8, len(radii)))

        for ax, kernel in zip(axes, kernels):
            df_k      = df_mode[df_mode['kernel'] == kernel]
            df_tot_k  = df_total[df_total['kernel'] == kernel].set_index('radius')

            data   = []
            labels = []

            for r in radii:
                d = df_k[df_k['radius'] == r]['pct_pruned'].values
                data.append(d)

                total_l  = df_tot_k.loc[r, 'total_leaves'] if r in df_tot_k.index else 0
                pruned_l = len(d)
                pct_l    = pruned_l / total_l * 100 if total_l > 0 else 0

                labels.append(
                    f'r={r}\n'
                    f'{pruned_l}/{total_l}\n'
                    f'({pct_l:.1f}% hojas)'
                )

            bp = ax.boxplot(data,
                            patch_artist=True,
                            medianprops=dict(color='white', linewidth=2),
                            whiskerprops=dict(linewidth=1.2),
                            capprops=dict(linewidth=1.2),
                            flierprops=dict(marker='o', markersize=2,
                                            alpha=0.3, linestyle='none'))

            for patch, color in zip(bp['boxes'], colors):
                patch.set_facecolor(color)

            for i, d in enumerate(data, start=1):
                if len(d) == 0:
                    continue
                median = np.median(d)
                mean   = np.mean(d)
                ax.text(i, median + 1, f'{median:.1f}%',
                        ha='center', va='bottom', fontsize=7,
                        color='white', fontweight='bold')
                ax.scatter(i, mean, marker='D', color='red',
                           s=20, zorder=5, label='media' if i == 1 else '')

            ax.set_xticks(range(1, len(radii) + 1))
            ax.set_xticklabels(labels, fontsize=8)
            ax.set_xlabel('Radio de búsqueda', fontsize=10)
            ax.set_title(kernel.upper(), fontsize=12, fontweight='bold')
            ax.grid(axis='y', alpha=0.3)
            ax.spines[['top', 'right']].set_visible(False)
            ax.set_ylim(0, 105)

            if ax == axes[0]:
                ax.set_ylabel('% Puntos podados', fontsize=11)
                ax.legend(fontsize=8)

        fig.suptitle(
            f'Distribución del % de poda por hoja\n'
            f'Dataset: {dataset_name}   Modo: {mode} Config: {configs}',
            fontsize=13, fontweight='bold', y=1.02)
        plt.show()
        if (save):
            out_path = Path(csv_path).parent / f'{dataset_name}_{mode}_pruning_distribution.png'
            plt.tight_layout()
            plt.savefig(out_path, dpi=150, bbox_inches='tight')
            plt.close()
            print(f'Guardado: {out_path}')
        plt.close(fig)

def plot_density_analysis(csv_path, save: bool = False) -> None:
    df = pd.read_csv(csv_path)
    dataset_name = Path(csv_path).stem

    # Detectar si hay columnas de configuración
    has_threshold = 'threshold' in df.columns
    has_max_leaf  = 'maxPointsLeaf' in df.columns

    # Construir etiqueta de configuración por fila
    def config_label(row):
        parts = []
        if has_threshold:
            parts.append(f'thr={int(row["threshold"])}')
        if has_max_leaf:
            parts.append(f'maxL={int(row["maxPointsLeaf"])}')
        return ' | '.join(parts) if parts else 'default'

    df['config'] = df.apply(config_label, axis=1)
    configs = df['config'].unique()
    
    # 1. Calcular el % de poda
    df['pruning_pct'] = (1 - df['count'] / df['total']) * 100
    
    # 2. Definir rangos de densidad
    bins = [0, 1, 16, 32, 64, 128, 256, 512, 1024, 10000]
    labels = ['0-1', '2-16', '17-32', '33-64', '65-128', '129-256', '257-512', '513-1024', '>1024']
    df['density_range'] = pd.cut(df['total'], bins=bins, labels=labels)
    
    kernels = sorted(df['kernel'].unique())
    n_kernels = len(kernels)
    
    # 3. Setup de la figura
    fig, axes = plt.subplots(1, n_kernels, figsize=(12 * n_kernels, 8), sharey=True)
    if n_kernels == 1: axes = [axes]

    # Paleta de colores para los radios
    radii = sorted(df['radius'].unique())
    modes = sorted(df['mode'].unique())
    combinations = list(product(radii, modes))

    palette = sns.color_palette("rocket", n_colors=len(combinations))

    for i, kernel in enumerate(kernels):
        ax1 = axes[i]
        df_k = df[df['kernel'] == kernel]
        
        # --- FONDO: Distribución general de hojas (Histograma) ---
        # Sumamos todas las hojas de todos los radios para ver la estructura del dataset
        summary_bg = df_k.groupby('density_range', observed=True)['leaf'].count().reset_index()
        sns.barplot(data=summary_bg, x='density_range', y='leaf', 
                    color='lightgray', alpha=0.3, ax=ax1, label='Densidad Dataset')
        
        ax1.set_title(f'Kernel: {kernel.upper()}', fontsize=18, fontweight='bold', pad=25)
        ax1.set_ylabel('Nº de Hojas (Total)', fontsize=14, color='gray')
        ax1.tick_params(axis='x', rotation=45)
        
        # --- SEGUNDO EJE: Series de Poda por Radio ---
        ax2 = ax1.twinx()
        
        for i, (r, mode) in enumerate(combinations):
            df_rad = df_k[df_k['radius'] == r]
            df_r = df_rad[df_rad['mode'] == mode]
            # Calculamos la poda media por cada rango de densidad para este radio concreto
            summary_r = df_r.groupby('density_range', observed=True)['pruning_pct'].mean().reset_index()
            
            # Dibujamos la serie
            sns.lineplot(data=summary_r, x='density_range', y='pruning_pct', 
                         ax=ax2, marker='o', markersize=8, linewidth=2.5,
                         label=f'r = {r}, mode = {mode}', color=palette[i])

        ax2.set_ylabel('Poda Media (%)', fontsize=14, fontweight='bold')
        ax2.set_ylim(-5, 105)
        
        ax1.grid(True, axis='y', linestyle='--', alpha=0.2)
        ax2.legend(title="Radio de Búsqueda", loc='upper left', frameon=True, shadow=True)

    plt.suptitle(f'Evolución de la Eficiencia: Densidad vs. Poda por Radio\nDataset: {dataset_name}   Config: {", ".join(configs)}', 
                 fontsize=22, fontweight='bold', y=1.05)
    
    plt.tight_layout()
    if save:
        plt.savefig(f"{dataset_name}_pro_density_analysis.png", dpi=200, bbox_inches='tight')
    plt.show()
    plt.close(fig)


def plot_key_distribution(csv_path: str, save: bool = False) -> None:
    df = pd.read_csv(csv_path)
    dataset_name = Path(csv_path).stem
    # Detectar si hay columnas de configuración
    has_threshold = 'threshold' in df.columns
    has_max_leaf  = 'maxPointsLeaf' in df.columns

    # Construir etiqueta de configuración por fila
    def config_label(row):
        parts = []
        if has_threshold:
            parts.append(f'thr={int(row["threshold"])}')
        if has_max_leaf:
            parts.append(f'maxL={int(row["maxPointsLeaf"])}')
        return ' | '.join(parts) if parts else 'default'

    df['config'] = df.apply(config_label, axis=1)
    configs = df['config'].unique()

    # Para cada hoja nos quedamos con la mejor clave
    best = df.loc[df.groupby(['leaf', 'kernel', 'mode', 'radius'])['count'].idxmin()].copy()

    key_names = {0: 'K0 (X)', 1: 'K1 (Y)', 2: 'K2 (Z)'}
    key_colors = {0: '#4472C4', 1: '#2E8B57', 2: '#90EE90'}

    best['pct_pruned'] = (1 - best['count'] / best['total']) * 100
    pruned = best[best['pct_pruned'] > 0]
    modes   = pruned['mode'].unique()
    kernels = sorted(pruned['kernel'].unique())
    radii   = sorted(pruned['radius'].unique())

    for mode in modes:
        if mode == 'polar':
            continue
        df_mode = pruned[pruned['mode'] == mode]

        fig, axes = plt.subplots(1, len(kernels),
                                  figsize=(5 * len(kernels), 5),
                                  sharey=True)
        if len(kernels) == 1:
            axes = [axes]

        for ax, kernel in zip(axes, kernels):
            df_k = df_mode[df_mode['kernel'] == kernel]

            # Contar hojas por clave y radio
            counts = (
                df_k.groupby(['radius', 'key'])
                .size()
                .unstack(fill_value=0)
                .reindex(columns=[0, 1, 2], fill_value=0)
            )
            # Normalizar a porcentaje
            pcts = counts.div(counts.sum(axis=1), axis=0) * 100

            bottom = np.zeros(len(radii))
            x = np.arange(len(radii))

            for key_id in [0, 1, 2]:
                vals = [pcts.loc[r, key_id] if r in pcts.index else 0 for r in radii]
                bars = ax.bar(x, vals, bottom=bottom,
                              label=key_names[key_id],
                              color=key_colors[key_id],
                              edgecolor='white', linewidth=0.5)
                # Etiqueta dentro de la barra si hay espacio
                for bar, val, bot in zip(bars, vals, bottom):
                    if val > 5:
                        ax.text(bar.get_x() + bar.get_width() / 2,
                                bot + val / 2,
                                f'{val:.1f}%',
                                ha='center', va='center',
                                fontsize=8, color='white', fontweight='bold')
                bottom += np.array(vals)

            ax.set_xticks(x)
            ax.set_xticklabels([f'r={r}' for r in radii], fontsize=9)
            ax.set_xlabel('Radio de búsqueda', fontsize=10)
            ax.set_title(kernel.upper(), fontsize=12, fontweight='bold')
            ax.set_ylim(0, 100)
            ax.set_ylabel('% hojas' if ax == axes[0] else '', fontsize=11)
            ax.grid(axis='y', alpha=0.2)
            ax.spines[['top', 'right']].set_visible(False)

            if ax == axes[0]:
                ax.legend(fontsize=8, loc='upper right')

        fig.suptitle(
            f'Distribución de clave elegida por hoja\n'
            f'Dataset: {dataset_name}   Modo: {mode}   Config: {", ".join(configs)}',
            fontsize=13, fontweight='bold', y=1.02)
        plt.show()
        if (save):
            out_path = Path(csv_path).parent / f'{dataset_name}_{mode}_key_distribution.png'
            plt.tight_layout()
            plt.savefig(out_path, dpi=150, bbox_inches='tight')
            plt.close()
            print(f'Guardado: {out_path}')
        plt.close(fig)



##########################################################
################ GRAFICAS DE DEBUG LEAVES ################
##########################################################

def analyze_speedup_and_timing(csv_path: str, save: bool = False, filename: str = ""):
    df = pd.read_csv(csv_path)
    if not filename:
        filename = Path(csv_path).stem
        filename = filename.split('-')[0] 
    
    # Detectar si hay columnas de configuración
    has_threshold = 'threshold' in df.columns
    has_max_leaf  = 'maxPointsLeaf' in df.columns

    # Construir etiqueta de configuración por fila
    def config_label(row):
        parts = []
        if has_threshold:
            parts.append(f'thr={int(row["threshold"])}')
        if has_max_leaf:
            parts.append(f'maxL={int(row["maxPointsLeaf"])}')
        return ' | '.join(parts) if parts else 'default'

    df['config'] = df.apply(config_label, axis=1)
    configs = df['config'].unique()
    
    avg_times = df.groupby(['kernel', 'radius', 'mode'])[['get_range_time', 'loop_time']].mean().reset_index()
    
    # Calcular Speedup por búsqueda individual
    reference = df[df['mode'] == 'none'].groupby(['kernel', 'radius'])['loop_time'].mean().reset_index()
    reference.rename(columns={'loop_time': 'loop_time_none'}, inplace=True)
    
    df_speedup = pd.merge(df, reference, on=['kernel', 'radius'])
    
    df_speedup['total_time'] = df_speedup['get_range_time'] + df_speedup['loop_time']
    
    df_speedup['speedup'] = df_speedup['loop_time_none'] / df_speedup['total_time']
    
    df_speedup_filtered = df_speedup[df_speedup['mode'] != 'none']

    # --- GRÁFICA 1: TIEMPO DESGLOSADO (Barras Apiladas) ---
    kernels = avg_times['kernel'].unique()
    for kern in kernels:
        df_kern = avg_times[avg_times['kernel'] == kern]
        
        modes = df_kern['mode'].unique()
        radii = sorted(df_kern['radius'].unique())
        
        fig, ax = plt.subplots(figsize=(12, 7))
        
        width = 0.25  # ancho de las barras
        multiplier = 0
        
        for mode in modes:
            df_m = df_kern[df_kern['mode'] == mode]
            # Asegurar orden de radios
            df_m = df_m.set_index('radius').reindex(radii).reset_index()
            
            offset = width * multiplier
            
            # Parte inferior: get_range_time
            p1 = ax.bar(np.arange(len(radii)) + offset, df_m['get_range_time'], width, 
                        label=f'{mode} (Selector)', alpha=0.6)
            
            # Parte superior: loop_time
            p2 = ax.bar(np.arange(len(radii)) + offset, df_m['loop_time'], width, 
                        bottom=df_m['get_range_time'], label=f'{mode} (Loop)', alpha=0.9)
            
            multiplier += 1
        ax.set_ylabel('Tiempo Medio (Nanosegundos)')
        ax.set_title(f'Desglose de Tiempo: Selector vs Loop\nKernel: {kern.upper()} | Dataset: {filename} | Config: {", ".join(configs)}')
        ax.set_xticks(np.arange(len(radii)) + width, [f'r={r}' for r in radii])
        ax.legend(loc='upper left', bbox_to_anchor=(1, 1))
        ax.grid(axis='y', alpha=0.3)
        
        plt.tight_layout()
        plt.show()
        if save:
            out_path = Path(csv_path).parent / f'{filename}_{kern}_time_breakdown.png'
            plt.savefig(out_path, dpi=150, bbox_inches='tight')
            plt.close()
            print(f'Guardado: {out_path}')

    # --- GRÁFICA 2: DISTRIBUCIÓN DEL SPEEDUP (Boxplot) ---
    for kern in kernels:
        df_kern_s = df_speedup_filtered[df_speedup_filtered['kernel'] == kern]
        
        plt.figure(figsize=(12, 7))
        sns.boxplot(data=df_kern_s, x='radius', y='speedup', hue='mode', palette='Set2')
        
        plt.axhline(y=1, color='red', linestyle='--', label='Break-even (Speedup=1)')
        
        plt.yscale('log')
        plt.title(f'Distribución de Speedup Individual por Radio\nKernel: {kern.upper()} | Dataset: {filename} | Config: {", ".join(configs)}')
        plt.ylabel('Speedup (Log Scale)')
        plt.xlabel('Radio de búsqueda')
        plt.legend()
        plt.grid(axis='y', alpha=0.3)
        
        plt.show()
        if save:
            out_path = Path(csv_path).parent / f'{filename}_{kern}_speedup_distribution.png'
            plt.savefig(out_path, dpi=150, bbox_inches='tight')
            plt.close()
            print(f'Guardado: {out_path}')
        plt.close(fig)


##########################################################
#################### GRAFICAS FINALES ####################
##########################################################
def plot_reorder_vs_base(df, cloud, algo, radius):
    """
    Crea dos subplots (Cube vs Sphere) para comparar la reordenación 
    frente a la base variando max_leaf.
    """
    # Filtramos por nube, algoritmo y radio
    subset = df[(df['octree'].str.contains(cloud)) & 
                (df['operation'] == algo) & 
                (df['radius'] == radius)].copy()
    
    # Etiquetamos experimentos
    subset['experiment'] = subset['reorder'].apply(lambda x: "Base" if x == 'none' else f"Reordered ({x})")
    
    # Creamos la figura con 2 subplots
    fig, axes = plt.subplots(1, 2, figsize=(16, 6), sharey=True)
    kernels = ['Cube', 'Sphere']
    
    for i, kernel in enumerate(kernels):
        ax = axes[i]
        data_kernel = subset[subset['kernel'] == kernel]
        
        if data_kernel.empty:
            ax.set_title(f"No hay datos para {kernel}")
            continue
            
        sns.lineplot(data=data_kernel, x='max_leaf', y='mean', hue='experiment', 
                     marker='o', linewidth=2.5, ax=ax)
        
        ax.set_title(f'Kernel: {kernel.upper()}', fontsize=14, fontweight='bold')
        ax.set_ylabel('Tiempo Medio (ns)')
        ax.set_xlabel('Puntos máximos por hoja (max_leaf)')
        ax.grid(True, which="both", ls="-", alpha=0.2)
        ax.set_yscale('log')
        
    plt.suptitle(f'Impacto de la Reordenación por Kernel ({cloud}, r={radius})', fontsize=16, y=1.02)
    plt.tight_layout()
    
    return fig

def plot_threshold_heatmap_comparison(df, cloud, reorder_mode, radius):
    """
    Genera dos mapas de calor (Cube vs Sphere) para encontrar el 
    umbral de poda (threshold) y max_leaf óptimos.
    """
    subset = df[(df['octree'].str.contains(cloud)) & 
                (df['reorder'] == reorder_mode) & 
                (df['radius'] == radius)]
    
    fig, axes = plt.subplots(1, 2, figsize=(18, 8), sharey=True)
    kernels = ['Cube', 'Sphere']
    
    # Buscamos el valor mínimo global para que la escala de colores sea comparable
    vmin = subset['mean'].min()
    vmax = subset['mean'].max()

    for i, kernel in enumerate(kernels):
        ax = axes[i]
        data_kernel = subset[subset['kernel'] == kernel]
        
        if data_kernel.empty:
            continue
            
        # Pivotar: Filas (Max Leaf), Columnas (Threshold)
        pivot = data_kernel.pivot_table(index='max_leaf', columns='threshold', 
                                        values='mean', aggfunc='mean')
        
        sns.heatmap(pivot, annot=True, fmt=".0f", cmap="YlGnBu_r", 
                    ax=ax, vmin=vmin, vmax=vmax, cbar_kws={'label': 'Tiempo (ns)'})
        
        ax.set_title(f'Kernel: {kernel.upper()}', fontsize=14, fontweight='bold')
        ax.set_xlabel('Umbral de Poda (threshold)')
        ax.set_ylabel('Max Leaf (puntos/hoja)')

    plt.suptitle(f'Optimización de Hiperparámetros - Modo: {reorder_mode}\nDataset: {cloud} | Radio: {radius}', 
                 fontsize=16, fontweight='bold', y=1.02)
    plt.tight_layout()
    
    return fig