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
from matplotlib.colors import LightSource


##########################################################
################ GRAFICAS DE DEBUG RANGES ################
##########################################################
import matplotlib.ticker as mticker
#
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

    # --- CÁLCULO DE L MEDIA SEGURO (SOLO MODO NONE) ---
    if 'L' in df.columns and 'mode' in df.columns:
        df_base_leaves = df[df['mode'] == 'none']
        if df_base_leaves.empty:
            df_base_leaves = df
            
        leaf_mapping = (
            df_base_leaves.groupby(['kernel', 'radius'])['L']
            .mean()
            .reset_index()
            .rename(columns={'L': 'L_clean'})
        )
    else:
        leaf_mapping = None

    # --- AGREGACIÓN DIRECTA Y LIMPIA ---
    # Sumamos directamente todas las pasadas del dataset para evitar que idxmin descarte filas clave
    agg = (
        df.groupby(['mode', 'kernel', 'radius', 'config'])
        .agg(
            total_pts=('total', 'sum'), 
            evaluated_pts=('count', 'sum'),
            neighbor_pts=('neighbours', 'sum')
        )
        .reset_index()
    )
    
    # Inyectamos de forma segura el radio de hoja medio calculado con el modo base
    if leaf_mapping is not None:
        agg = pd.merge(agg, leaf_mapping, on=['kernel', 'radius'], how='left')
    else:
        agg['L_clean'] = 1.0

    # Porcentajes de eficiencia de poda
    agg['pct_evaluated'] = (agg['evaluated_pts'] / agg['total_pts']) * 100
    agg['pct_neighbors_total'] = (agg['neighbor_pts'] / agg['total_pts']) * 100

    modes   = agg['mode'].unique()
    kernels = sorted(agg['kernel'].unique())
    radii   = sorted(agg['radius'].unique())

    for mode in modes:
        for config in configs:
            df_mode = agg[(agg['mode'] == mode) & (agg['config'] == config)]
            if df_mode.empty: continue

            n_kernels = len(kernels)
            n_radii   = len(radii)
            
            # Ajuste de proporciones para el bloque tricolor de barras
            group_width = 0.85
            bar_width   = group_width / (n_radii * 3.5) 

            x = np.arange(n_kernels)
            colors_total = plt.cm.Blues(np.linspace(0.4, 0.7, n_radii))
            colors_eval  = plt.cm.Oranges(np.linspace(0.4, 0.7, n_radii))
            colors_neigh = plt.cm.Greens(np.linspace(0.4, 0.7, n_radii))

            fig, ax = plt.subplots(figsize=(max(12, n_kernels * 3), 7))

            for ri, radius in enumerate(radii):
                df_r = df_mode[df_mode['radius'] == radius].set_index('kernel')

                # Centro de masa del subgrupo de radio para la alineación del texto inferior
                base_offset = (ri - (n_radii - 1) / 2) * (bar_width * 3.8)
                
                offset_total = base_offset - bar_width
                offset_eval  = base_offset
                offset_neigh = base_offset + bar_width

                totals    = [df_r.loc[k, 'total_pts']     if k in df_r.index else 0 for k in kernels]
                evaluated = [df_r.loc[k, 'evaluated_pts'] if k in df_r.index else 0 for k in kernels]
                neighbors = [df_r.loc[k, 'neighbor_pts']  if k in df_r.index else 0 for k in kernels]
                
                pct_eval  = [df_r.loc[k, 'pct_evaluated']       if k in df_r.index else 0 for k in kernels]
                pct_neigh = [df_r.loc[k, 'pct_neighbors_total'] if k in df_r.index else 0 for k in kernels]

                # Renderizado de barras
                ax.bar(x + offset_total, totals, bar_width, label=f'r={radius} Total' if ri==0 else "",
                       color=colors_total[ri], edgecolor='black', linewidth=0.3)
                
                bars_ev = ax.bar(x + offset_eval, evaluated, bar_width, label=f'r={radius} Evaluados' if ri==0 else "",
                                 color=colors_eval[ri], edgecolor='black', linewidth=0.3)
                
                bars_ne = ax.bar(x + offset_neigh, neighbors, bar_width, label=f'r={radius} Vecinos' if ri==0 else "",
                                 color=colors_neigh[ri], edgecolor='black', linewidth=0.3)

                # --- TEXTO R/L INDEPENDIENTE Y SEGURO ---
                for ki, k in enumerate(kernels):
                    if k in df_r.index:
                        L_val = df_r.loc[k, 'L_clean']
                        ratio_rl = radius / L_val if L_val > 0 else radius
                        
                        pos_x = ki + base_offset
                        
                        ax.text(pos_x, -0.04, f'R/L\n{ratio_rl:.2f}', 
                                transform=ax.get_xaxis_transform(),
                                ha='center', va='top', fontsize=7.5, color='#555555')

                # Etiquetas numéricas de rendimiento sobre las barras (Evita superposiciones)
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
            ax.set_xticklabels([k.upper() for k in kernels], fontsize=11, fontweight='bold')
            ax.tick_params(axis='x', pad=35) # Deja espacio suficiente para el bloque de texto R/L

            ax.set_ylabel('Nº de Puntos', fontsize=12)
            ax.yaxis.set_major_formatter(mticker.FuncFormatter(
                lambda val, _: f'{val/1e6:.1f}M' if val >= 1e6 else f'{val/1e3:.0f}K' if val >= 1e3 else f'{val:.0f}'))
            
            ax.set_title(f'Puntos: Total vs Evaluados (Poda) vs Vecinos Reales\n'
                         f'Dataset: {dataset_name} | Modo: {mode} | {config}', fontsize=12, fontweight='bold')
            
            # Quitar etiquetas repetidas de la leyenda
            handles, labels = ax.get_legend_handles_labels()
            by_label = dict(zip(labels, handles))
            ax.legend(by_label.values(), by_label.keys(), loc='upper left', bbox_to_anchor=(1, 1), fontsize=8, ncol=1)
            ax.grid(axis='y', which='both', alpha=0.2)
            
            plt.tight_layout()
            plt.show()

            if save:
                safe_config = config.replace(' | ', '_').replace('=', '')
                out_path = Path(csv_path).parent / f'{dataset_name}_{mode}_{safe_config}_efficiency.png'
                fig.savefig(out_path, dpi=150, bbox_inches='tight')
                print(f'Guardado: {out_path}')
            
            plt.close(fig)

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
        ax.set_ylabel('Tiempo Medio (Segundos)')
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
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import matplotlib.ticker as ticker

def plot_reorder_vs_base(data_path, cloud, allFiles=False, kernel="all", radius="all"):
    """
    Consolida todos los archivos en un único DataFrame y genera una figura independiente
    para cada valor de radio, mostrando 'maxPointsLeaf' en el eje X con gráficos de barras.
    Devuelve una lista de objetos Figure de Matplotlib.
    """
    # 1. Consolidar DataFrames
    if allFiles:
        dfs_list = get_dataset_files_in_dir(data_path, cloud)
    else:
        single_df = get_dataset_file(data_path, cloud)
        dfs_list = [single_df] if single_df is not None else []

    dfs_list = [d for d in dfs_list if d is not None and not d.empty]
    if not dfs_list:
        print(f"Advertencia: No se encontraron datos válidos para la nube {cloud}.")
        return []

    df_global = pd.concat(dfs_list, ignore_index=True)

    # Filtros de Kernel y Radio
    if kernel != "all":
        if isinstance(kernel, str): kernel = [kernel]
        df_global = df_global[df_global["kernel"].isin(kernel)]
        
    if radius != "all":
        if not isinstance(radius, list): radius = [radius]
        df_global = df_global[df_global["radius"].isin(radius)]

    if df_global.empty:
        return []

    # Dejamos los nombres originales de tus experimentos tal y como vienen mapeados en tu script
    df_global['experiment'] = df_global['reorder'].apply(lambda x: "Base" if x == 'none' else f"Reordered ({x})")

    # Mapeo TeX para la notación matemática formal de los Kernels
    kernel_titles = {
        'circle': r'$\mathcal{N}_{Circle}$',
        'sphere': r'$\mathcal{N}_{Sphere}$',
        'square': r'$\mathcal{N}_{Square}$',
        'cube': r'$\mathcal{N}_{Cube}$'
    }

    unique_radii = sorted(df_global['radius'].unique())
    unique_kernels = df_global['kernel'].unique()

    figs = []  

    # 2. Iterar por cada RADIO
    for rad in unique_radii:
        df_rad = df_global[df_global['radius'] == rad].copy()
        if df_rad.empty: continue

        n_kernels = len(unique_kernels)
        
        # Creamos los subplots compartiendo el eje Y
        fig, axes = plt.subplots(1, n_kernels, figsize=(6.5 * n_kernels, 5), sharey=True, squeeze=False)
        axes = axes.flatten()
        
        # Guardaremos los handles de las barras para crear la leyenda global
        handles, labels = [], []
        
        for i, kern in enumerate(unique_kernels):
            ax = axes[i]
            data_kernel = df_rad[df_rad['kernel'] == kern]
            
            if data_kernel.empty:
                ax.set_title(f"No hay datos para {kern}")
                continue
                
            # Renderizado del gráfico de barras
            bp = sns.barplot(
                data=data_kernel, 
                x='maxPointsLeaf', 
                y='mean', 
                hue='experiment', 
                ax=ax,
                edgecolor='#444444',
                linewidth=0.8,
                alpha=0.85
            )
            
            # Recuperamos los handles de la leyenda de este ax antes de eliminarla
            if not handles and ax.get_legend_handles_labels()[0]:
                handles, labels = ax.get_legend_handles_labels()
            if ax.get_legend() is not None:
                ax.get_legend().remove() 
            
            # --- MODIFICACIÓN: Título con Radio y Notación del Kernel ---
            tex_kernel = kernel_titles.get(kern.lower(), f'{kern.upper()}')
            ax.set_title(f'{tex_kernel}\n' r'$r = ' f'{rad}\ m$', fontsize=13, pad=10)
            
            # Estilización de ejes
            ax.set_ylabel('Mean runtime (s)' if i == 0 else '', fontsize=12)
            ax.set_xlabel('Puntos máximos por hoja (maxPointsLeaf)', fontsize=11)
            ax.grid(True, which="both", ls=":", alpha=0.15, axis='y')
            ax.set_yscale('log')
            
            # Formateador del eje Y logarítmico limpio
            ax.yaxis.set_major_locator(ticker.LogLocator(base=10.0, subs=(1.0, 2.0, 5.0), numticks=20))
            ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda y, _: f'{y:g}'))
            ax.tick_params(axis='both', which='major', labelsize=10)

        # --- LEYENDA ÚNICA GLOBAL SUPERIOR ---
        if handles:
            fig.legend(
                handles, labels,
                loc="upper center",
                bbox_to_anchor=(0.5, 0.98),  # Centrado en la parte superior de la figura
                ncol=len(labels),             # Una sola fila con todas las opciones
                frameon=True,
                facecolor='white',
                edgecolor='#CCCCCC',
                fontsize=11,
                handlelength=1.8,
                handletextpad=0.5,
                columnspacing=1.5
            )
        
        # Ajuste de márgenes para dar espacio arriba a la leyenda y los títulos
        plt.subplots_adjust(left=0.08, right=0.95, bottom=0.15, top=0.78, wspace=0.15)
        
        figs.append(fig)
        
    return figs

def plot_threshold_heatmap_comparison(data_path, cloud, reorder_mode, allFiles=True, kernel="all", radius="all"):
    """
    Genera figuras independientes por cada valor de radio. Cada figura contiene subplots
    en forma de mapas de calor para encontrar la combinación óptima de 'threshold' y 'maxPointsLeaf'.
    Los títulos de cada gráfico muestran la notación matemática formal del kernel y el radio.
    Devuelve una lista de objetos Figure de Matplotlib.
    """
    # 1. Consolidar DataFrames
    if allFiles:
        dfs_list = get_dataset_files_in_dir(data_path, cloud)
    else:
        single_df = get_dataset_file(data_path, cloud)
        dfs_list = [single_df] if single_df is not None else []

    dfs_list = [d for d in dfs_list if d is not None and not d.empty]
    if not dfs_list:
        print(f"Advertencia: No se encontraron datos válidos para la nube {cloud}.")
        return []

    df_global = pd.concat(dfs_list, ignore_index=True)
    mask = (df_global['reorder'] == reorder_mode)
    df_filtered = df_global[mask].copy()

    if kernel != "all":
        if isinstance(kernel, str): kernel = [kernel]
        df_filtered = df_filtered[df_filtered["kernel"].isin(kernel)]
        
    if radius != "all":
        if not isinstance(radius, list): radius = [radius]
        df_filtered = df_filtered[df_filtered["radius"].isin(radius)]

    if df_filtered.empty:
        print(f"Advertencia: No hay datos que coincidan con los filtros aplicados para {cloud}.")
        return []

    # Mapeo TeX para la notación matemática formal de los Kernels
    kernel_titles = {
        'circle': r'$\mathcal{N}_{Circle}$',
        'sphere': r'$\mathcal{N}_{Sphere}$',
        'square': r'$\mathcal{N}_{Square}$',
        'cube': r'$\mathcal{N}_{Cube}$'
    }

    # Extraer los radios y kernels únicos presentes tras los filtros
    unique_radii = sorted(df_filtered['radius'].unique())
    unique_kernels = df_filtered['kernel'].unique()

    figs = []  # Array para acumular las figuras (una por cada radio)

    # 2. Bucle exterior: Una figura diferente por cada valor de RADIO
    for rad in unique_radii:
        df_rad = df_filtered[df_filtered['radius'] == rad].copy()
        if df_rad.empty: 
            continue

        n_kernels = len(unique_kernels)
        
        # Estructura de subplots en horizontal compartiendo el eje Y (maxPointsLeaf)
        fig, axes = plt.subplots(1, n_kernels, figsize=(7.5 * n_kernels, 5.5), sharey=True, squeeze=False)
        axes = axes.flatten()
        
        # Buscamos los límites mínimos y máximos específicos de este radio 
        # para que la escala de color (colorbar) sea perfectamente comparable entre subplots
        vmin = df_rad['mean'].min()
        vmax = df_rad['mean'].max()
        
        for i, kern in enumerate(unique_kernels):
            ax = axes[i]
            data_kernel = df_rad[df_rad['kernel'] == kern]
            
            if data_kernel.empty:
                ax.set_title(f"No hay datos para {kern}")
                continue
                
            # Pivotar datos: Filas (maxPointsLeaf), Columnas (threshold), Celda (mean)
            # Se usa 'maxPointsLeaf' en el index para mantener concordancia de nombres
            pivot = data_kernel.pivot_table(index='maxPointsLeaf', columns='threshold', 
                                            values='mean', aggfunc='mean')
            
            # Dibujar el mapa de calor
            # Solo pintamos la barra de color (cbar) en el último mapa de la derecha para no saturar
            show_cbar = (i == n_kernels - 1)
            
            sns.heatmap(
                pivot, 
                annot=True, 
                fmt=".2f",          # Dos decimales para los tiempos medios (puedes poner .0f si son enteros)
                cmap="coolwarm_r", 
                ax=ax, 
                vmin=vmin, 
                vmax=vmax, 
                cbar=show_cbar,
                cbar_kws={'label': 'Tiempo Total (s)'} if show_cbar else None,
                linewidths=0.5,
                linecolor='#EEEEEE'
            )
            
            # --- Título con Radio y Notación del Kernel ---
            tex_kernel = kernel_titles.get(kern.lower(), f'{kern.upper()}')
            ax.set_title(f'{tex_kernel}\n' r'$r = ' f'{rad}\ m$', fontsize=13, pad=10)
            
            # Configuración de los nombres de los ejes
            ax.set_xlabel('Umbral de Poda (threshold)', fontsize=11)
            ax.set_ylabel('Puntos máximos por hoja (maxPointsLeaf)' if i == 0 else '', fontsize=11)
            
            # Forzar a que las etiquetas del eje Y no se roten de forma extraña
            ax.tick_params(axis='y', rotation=0)

        # Ajuste preciso de los márgenes de la figura para evitar solapamientos
        plt.subplots_adjust(left=0.10, right=0.92, bottom=0.15, top=0.82, wspace=0.18)
        plt.suptitle(
            f'Comparación de Configuraciones de Poda\n'
            f'Dataset: {cloud} | Reorder: {reorder_mode}',
            fontsize=16, fontweight='bold', y=1.02)
        plt.show()
        figs.append(fig)
        
    return figs

##########################################################
######### GRAFICAS DE VIS. PODAS EN ESPACIOS 3D ##########
##########################################################
def plot_octree_pruning_polar(save: bool = False):
    # Creamos una figura con 2 subgráficas en paralelo
    fig = plt.figure(figsize=(18, 8))
    
    # 1. Configuración del Nodo del Octree (un cubo de ejemplo)
    cube_bounds = [1.0, 3.0, 1.0, 3.0, 0.0, 2.0]
    cx = (cube_bounds[0] + cube_bounds[1]) / 2
    cy = (cube_bounds[2] + cube_bounds[3]) / 2
    cz = (cube_bounds[4] + cube_bounds[5]) / 2
    
    # 2. Configuración del Kernel (Esfera de consulta)
    q_x, q_y, q_z = 3.5, 3.5, 1.0  
    r = 1.8                         
    
    # 3. Cálculo de los límites de Poda Angulares
    dx = q_x - cx  
    dy = q_y - cy
    dxy = np.sqrt(dx**2 + dy**2)
    rxyEff = r 
    
    phiQ = np.arctan2(dy, dx)
    if phiQ < 0:
        phiQ += 2 * np.pi
        
    deltaPhi = np.arcsin(np.clip(rxyEff / dxy, 0.0, 1.0))
    kMinRaw = phiQ - deltaPhi
    kMaxRaw = phiQ + deltaPhi

    # 4. Generación de las mallas de los Hiperplanos con TRASLACIÓN
    r_lines = np.linspace(0, 4.0, 10)
    # Hacemos que Z cubra el rango del cubo y un poco más
    z_lines = np.linspace(cube_bounds[4] - 0.5, cube_bounds[5] + 0.5, 10)
    R, Z = np.meshgrid(r_lines, z_lines)
    
    # Hiperplano Mínimo trasladado al centro (cx, cy)
    X_min = cx + R * np.cos(kMinRaw)
    Y_min = cy + R * np.sin(kMinRaw)
    
    # Hiperplano Máximo trasladado al centro (cx, cy)
    X_max = cx + R * np.cos(kMaxRaw)
    Y_max = cy + R * np.sin(kMaxRaw)

    # Datos para la superficie de la esfera
    u, v = np.mgrid[0:2*np.pi:30j, 0:np.pi:20j]
    xs = q_x + r * np.cos(u) * np.sin(v)
    ys = q_y + r * np.sin(u) * np.sin(v)
    zs = q_z + r * np.cos(v)

    # =========================================================================
    # SUBPLOT 1: VISTA EN PERSPECTIVA 3D
    # =========================================================================
    ax1 = fig.add_subplot(121, projection='3d')
    
    # Dibujar aristas del Octree
    for x in cube_bounds[:2]:
        for y in cube_bounds[2:4]:
            ax1.plot([x, x], [y, y], cube_bounds[4:], color='black', lw=1.5)
    for x in cube_bounds[:2]:
        for z in cube_bounds[4:]:
            ax1.plot([x, x], cube_bounds[2:4], [z, z], color='black', lw=1.5)
    for y in cube_bounds[2:4]:
        for z in cube_bounds[4:]:
            ax1.plot(cube_bounds[:2], [y, y], [z, z], color='black', lw=1.5)
            
    ax1.scatter(cx, cy, cz, color='red', s=60, label='Centro del Nodo (Origen Polar)')
    ax1.scatter(q_x, q_y, q_z, color='blue', s=50, label='Centro Kernel (Q)')
    
    # Superficies
    ax1.plot_surface(xs, ys, zs, color='cyan', alpha=0.2, edgecolor='none')
    ax1.plot_surface(X_min, Y_min, Z, color='salmon', alpha=0.4, edgecolor='red', lw=0.3)
    ax1.plot_surface(X_max, Y_max, Z, color='salmon', alpha=0.4, edgecolor='red', lw=0.3)

    # Líneas guía (Saliendo desde el centro cx, cy, cz)
    ax1.plot([cx, cx + 4.0*np.cos(kMinRaw)], [cy, cy + 4.0*np.sin(kMinRaw)], [cz, cz], color='red', linestyle='--', lw=2, label='Límites de Poda (Φ)')
    ax1.plot([cx, cx + 4.0*np.cos(kMaxRaw)], [cy, cy + 4.0*np.sin(kMaxRaw)], [cz, cz], color='red', linestyle='--')
    ax1.plot([cx, q_x], [cy, q_y], [cz, q_z], color='blue', linestyle=':', lw=2, label='Eje Central Φ_Q')

    ax1.set_xlabel('Eje X')
    ax1.set_ylabel('Eje Y')
    ax1.set_zlabel('Eje Z')
    ax1.set_title('Vista Perspectiva 3D', fontsize=12)
    ax1.set_xlim(0.0, 6.0)
    ax1.set_ylim(0.0, 6.0)
    ax1.set_zlim(-0.5, 3.0)
    ax1.view_init(elev=25, azim=-55)
    ax1.legend(loc='upper left')

    # =========================================================================
    # SUBPLOT 2: VISTA CENITAL (DESDE ARRIBA)
    # =========================================================================
    ax2 = fig.add_subplot(122, projection='3d')
    
    # Replicamos el dibujo exacto en el segundo eje
    for x in cube_bounds[:2]:
        for y in cube_bounds[2:4]:
            ax2.plot([x, x], [y, y], cube_bounds[4:], color='black', lw=1.5)
    for x in cube_bounds[:2]:
        for z in cube_bounds[4:]:
            ax2.plot([x, x], cube_bounds[2:4], [z, z], color='black', lw=1.5)
    for y in cube_bounds[2:4]:
        for z in cube_bounds[4:]:
            ax2.plot(cube_bounds[:2], [y, y], [z, z], color='black', lw=1.5)
            
    ax2.scatter(cx, cy, cz, color='red', s=60)
    ax2.scatter(q_x, q_y, q_z, color='blue', s=50)
    ax2.plot_surface(xs, ys, zs, color='cyan', alpha=0.2, edgecolor='none')
    ax2.plot_surface(X_min, Y_min, Z, color='salmon', alpha=0.4, edgecolor='red', lw=0.3)
    ax2.plot_surface(X_max, Y_max, Z, color='salmon', alpha=0.4, edgecolor='red', lw=0.3)

    ax2.plot([cx, cx + 4.0*np.cos(kMinRaw)], [cy, cy + 4.0*np.sin(kMinRaw)], [cz, cz], color='red', linestyle='--', lw=2)
    ax2.plot([cx, cx + 4.0*np.cos(kMaxRaw)], [cy, cy + 4.0*np.sin(kMaxRaw)], [cz, cz], color='red', linestyle='--')
    ax2.plot([cx, q_x], [cy, q_y], [cz, q_z], color='blue', linestyle=':', lw=2)

    ax2.set_xlabel('Eje X')
    ax2.set_ylabel('Eje Y')
    ax2.set_zlabel('')  # Ocultamos la etiqueta Z en la vista cenital
    ax2.set_title('Vista Cenital (Plano XY)', fontsize=12)
    ax2.set_xlim(0.0, 6.0)
    ax2.set_ylim(0.0, 6.0)
    ax2.set_zlim(-0.5, 3.0)
    
    # CONFIGURACIÓN CRUCIAL: Elevación a 90 grados y azimut a -90 hace que miremos desde arriba perfectamente alineados
    ax2.view_init(elev=90, azim=-90)
    # Ocultamos los ticks y valores de Z para que parezca un plano 2D limpio
    ax2.set_zticklabels([])

    # =========================================================================
    # SALIDA
    # =========================================================================
    plt.suptitle('Poda Angular desde el centro de la hoja mediante el ángulo azimutal', fontsize=14, weight='bold', y=0.95)
    plt.subplots_adjust()

    if save:
        # Corregido: Usamos ruta relativa sin la barra inclinada inicial para evitar problemas de permisos de root
        output_dir = './images'
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        output_path = os.path.join(output_dir, 'poda_octree_polar.png')
        plt.savefig(output_path, dpi=300, bbox_inches='tight')
        plt.close()
        print(f"Gráfica doble guardada exitosamente en: {output_path}")
    else:
        plt.show()

def plot_octree_pruning_cartesian_x(save: bool = False):
    fig = plt.figure(figsize=(18, 8))
    
    # 1. Configuración del Nodo del Octree (El mismo cubo de antes)
    # Límites del cubo [xmin, xmax, ymin, ymax, zmin, zmax]
    cube_bounds = [1.0, 3.0, 1.0, 3.0, 0.0, 2.0]
    cx = (cube_bounds[0] + cube_bounds[1]) / 2
    cy = (cube_bounds[2] + cube_bounds[3]) / 2
    cz = (cube_bounds[4] + cube_bounds[5]) / 2
    
    node_xmin, node_xmax = cube_bounds[0], cube_bounds[1]
    node_ymin, node_ymax = cube_bounds[2], cube_bounds[3]
    node_zmin, node_zmax = cube_bounds[4], cube_bounds[5]

    # 2. Configuración del Kernel CÚBICO (AABB de consulta)
    # Lo posicionamos desplazado a la derecha en X para que corte el plano máximo del nodo
    kernel_size = 1.6
    kernel_cx, kernel_cy, kernel_cz = 3.4, 2.3, 1.0  # Centro del kernel
    
    k_xmin = kernel_cx - kernel_size / 2
    k_xmax = kernel_cx + kernel_size / 2
    k_ymin = kernel_cy - kernel_size / 2
    k_ymax = kernel_cy + kernel_size / 2
    k_zmin = kernel_cz - kernel_size / 2
    k_zmax = kernel_cz + kernel_size / 2
    
    kernel_bounds = [k_xmin, k_xmax, k_ymin, k_ymax, k_zmin, k_zmax]

    # 3. Lógica de Poda en el Eje X (Simulando tu código C++)
    # Evaluamos si los límites del kernel caen dentro del dominio [node_xmin, node_xmax]
    planes_x = []
    plane_labels = []
    
    if node_xmin <= k_xmin <= node_xmax:
        planes_x.append(k_xmin)
        plane_labels.append(f'Hiperplano Poda: X_min de Kernel ({k_xmin:.2f})')
        
    if node_xmin <= k_xmax <= node_xmax:
        planes_x.append(k_xmax)
        plane_labels.append(f'Hiperplano Poda: X_max de Kernel ({k_xmax:.2f})')

    # Generación de la malla para el hiperplano cartesiano (paralelo al plano YZ)
    # El plano se extenderá un poco más allá de los límites del nodo para que sea visible
    y_lines = np.linspace(node_ymin - 0.5, node_ymax + 0.5, 10)
    z_lines = np.linspace(node_zmin - 0.5, node_zmax + 0.5, 10)
    Y_plane, Z_plane = np.meshgrid(y_lines, z_lines)

    # Bucle para renderizar ambos subplots (1: Perspectiva, 2: Cenital)
    for subplot_idx, title in [(121, 'Vista Perspectiva 3D (Poda Eje X)'), (122, 'Vista Cenital (Plano XY)')]:
        ax = fig.add_subplot(subplot_idx, projection='3d')
        
        # --- DIBUJAR NODO (OCTREE) ---
        for x in cube_bounds[:2]:
            for y in cube_bounds[2:4]:
                ax.plot([x, x], [y, y], cube_bounds[4:], color='black', lw=1.5)
        for x in cube_bounds[:2]:
            for z in cube_bounds[4:]:
                ax.plot([x, x], cube_bounds[2:4], [z, z], color='black', lw=1.5)
        for y in cube_bounds[2:4]:
            for z in cube_bounds[4:]:
                ax.plot(cube_bounds[:2], [y, y], [z, z], color='black', lw=1.5)
                
        ax.scatter(cx, cy, cz, color='red', s=60, label='Centro del Nodo' if subplot_idx == 121 else "")

        # --- DIBUJAR KERNEL CÚBICO ---
        for x in kernel_bounds[:2]:
            for y in kernel_bounds[2:4]:
                ax.plot([x, x], [y, y], kernel_bounds[4:], color='blue', lw=1.2, alpha=0.8)
        for x in kernel_bounds[:2]:
            for z in kernel_bounds[4:]:
                ax.plot([x, x], kernel_bounds[2:4], [z, z], color='blue', lw=1.2, alpha=0.8)
        for y in kernel_bounds[2:4]:
            for z in kernel_bounds[4:]:
                ax.plot(kernel_bounds[:2], [y, y], [z, z], color='blue', lw=1.2, alpha=0.8, label='Kernel Cúbico (AABB)' if (subplot_idx == 121 and x == kernel_bounds[0] and y == kernel_bounds[2]) else "")
                
        ax.scatter(kernel_cx, kernel_cy, kernel_cz, color='blue', s=40)

        # --- DIBUJAR HIPERPLANOS DE PODA SI EXISTEN ---
        for px, label in zip(planes_x, plane_labels):
            X_plane = np.full_like(Y_plane, px)
            ax.plot_surface(X_plane, Y_plane, Z_plane, color='salmon', alpha=0.5, edgecolor='red', lw=0.5, 
                            label=label if subplot_idx == 121 else "")
            
            # Línea de trazo en la base para enfatizar el corte en X
            ax.plot([px, px], [node_ymin - 0.5, node_ymax + 0.5], [cz, cz], color='red', linestyle='--', lw=2)

        # Estética de los ejes
        ax.set_xlabel('Eje X (Dimensión de Poda)')
        ax.set_ylabel('Eje Y')
        if subplot_idx == 121:
            ax.set_zlabel('Eje Z')
            ax.view_init(elev=25, azim=-55)
            ax.legend(loc='upper left')
        else:
            ax.set_zlabel('')
            ax.set_zticklabels([])
            ax.view_init(elev=90, azim=-90) # Orientación de cámara cenital

        ax.set_title(title, fontsize=12)
        ax.set_xlim(0.0, 6.0)
        ax.set_ylim(0.0, 6.0)
        ax.set_zlim(-0.5, 3.0)

    plt.suptitle('Poda Cartesiana mediante el Eje X', fontsize=14, weight='bold', y=0.95)
    plt.subplots_adjust()

    if save:
        output_dir = './images'
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        output_path = os.path.join(output_dir, 'poda_octree_cartesiana_x.png')
        plt.savefig(output_path, dpi=300, bbox_inches='tight')
        plt.close()
        print(f"Gráfica cartesiana guardada exitosamente en: {output_path}")
    else:
        plt.show()
