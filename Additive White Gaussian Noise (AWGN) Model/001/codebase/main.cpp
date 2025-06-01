#include <gtk/gtk.h>
#include "SignalGenerator.hpp"
#include "AWGN.hpp"
#include "Analyzer.hpp"
#include "PlotWidget.hpp"

struct AppWidgets {
    GtkWidget *window;
    GtkWidget *amplitude_entry;
    GtkWidget *frequency_entry;
    GtkWidget *samples_entry;
    GtkWidget *snr_entry;
    GtkWidget *bandwidth_entry;
    GtkWidget *seed_entry;
    GtkWidget *generate_button;
    GtkWidget *reset_button;
    GtkWidget *notebook;
    GtkWidget *signal_plot;
    GtkWidget *time_plot;
    GtkWidget *time_label;
    GtkWidget *phasor_plot;
    GtkWidget *phasor_label;
};

static void show_error_dialog(GtkWidget *window, const char *message) {
    GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", message);
    gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
    gtk_alert_dialog_set_default_button(dialog, 0);
    gtk_alert_dialog_set_modal(dialog, TRUE);
    gtk_alert_dialog_show(dialog, GTK_WINDOW(window));
    g_object_unref(dialog);
}

static void reset_inputs(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = static_cast<AppWidgets*>(user_data);
    gtk_editable_set_text(GTK_EDITABLE(widgets->amplitude_entry), "1.0");
    gtk_editable_set_text(GTK_EDITABLE(widgets->frequency_entry), "0.05");
    gtk_editable_set_text(GTK_EDITABLE(widgets->samples_entry), "1000");
    gtk_editable_set_text(GTK_EDITABLE(widgets->snr_entry), "10.0");
    gtk_editable_set_text(GTK_EDITABLE(widgets->bandwidth_entry), "0.1");
    gtk_editable_set_text(GTK_EDITABLE(widgets->seed_entry), "0");
    gtk_label_set_text(GTK_LABEL(widgets->time_label), "Zero Crossings: N/A");
    gtk_label_set_text(GTK_LABEL(widgets->phasor_label), "Phasor Statistics: N/A");
    PlotWidget *signal_plot = PLOT_WIDGET(widgets->signal_plot);
    PlotWidget *time_plot = PLOT_WIDGET(widgets->time_plot);
    PlotWidget *phasor_plot = PLOT_WIDGET(widgets->phasor_plot);
    plot_widget_set_data(signal_plot, std::vector<double>(), std::vector<double>(), PLOT_TYPE_SIGNAL, 0);
    plot_widget_set_data(time_plot, std::vector<double>(), std::vector<double>(), PLOT_TYPE_TIME, 0);
    plot_widget_set_data(phasor_plot, std::vector<double>(), std::vector<double>(), PLOT_TYPE_PHASOR, 0);
    gtk_widget_queue_draw(widgets->signal_plot);
    gtk_widget_queue_draw(widgets->time_plot);
    gtk_widget_queue_draw(widgets->phasor_plot);
}

static void generate_signals(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = static_cast<AppWidgets*>(user_data);

    // Get parameters
    double amplitude = atof(gtk_editable_get_text(GTK_EDITABLE(widgets->amplitude_entry)));
    double frequency = atof(gtk_editable_get_text(GTK_EDITABLE(widgets->frequency_entry)));
    size_t num_samples = atoi(gtk_editable_get_text(GTK_EDITABLE(widgets->samples_entry)));
    double snr_db = atof(gtk_editable_get_text(GTK_EDITABLE(widgets->snr_entry)));
    double bandwidth = atof(gtk_editable_get_text(GTK_EDITABLE(widgets->bandwidth_entry)));
    unsigned int seed = atoi(gtk_editable_get_text(GTK_EDITABLE(widgets->seed_entry)));

    // Validate inputs
    if (amplitude <= 0) {
        show_error_dialog(widgets->window, "Amplitude must be greater than 0");
        return;
    }
    if (frequency <= 0) {
        show_error_dialog(widgets->window, "Frequency must be greater than 0");
        return;
    }
    if (num_samples == 0 || num_samples > 100000) {
        show_error_dialog(widgets->window, "Number of samples must be between 1 and 100,000");
        return;
    }
    if (snr_db < 0) {
        show_error_dialog(widgets->window, "SNR must be non-negative");
        return;
    }
    if (bandwidth <= 0) {
        show_error_dialog(widgets->window, "Bandwidth must be greater than 0");
        return;
    }

    // Generate signal and add noise
    SignalGenerator sig_gen(num_samples, amplitude, frequency);
    AWGN awgn(snr_db, seed);
    Analyzer analyzer;

    std::vector<double> signal = sig_gen.generateSineWave();
    std::vector<double> noisy_signal = awgn.addNoise(signal);
    double measured_snr = analyzer.computeSNR(signal, noisy_signal);
    double zero_crossings = analyzer.computeZeroCrossings(noisy_signal, frequency, bandwidth, snr_db);
    auto phasor_stats = analyzer.computePhasorStatistics(noisy_signal, signal, seed);
    double sigma1 = std::get<0>(phasor_stats);
    double sigma2 = std::get<1>(phasor_stats);
    double sigma3 = std::get<2>(phasor_stats);

    // Update time domain label
    char time_text[100];
    snprintf(time_text, sizeof(time_text), "Zero Crossings per Second: %.2f", zero_crossings);
    gtk_label_set_text(GTK_LABEL(widgets->time_label), time_text);

    // Update phasor label
    char phasor_text[200];
    snprintf(phasor_text, sizeof(phasor_text),
             "Phasor Statistics: %.1f%% in 1σ, %.1f%% in 2σ, %.1f%% in 3σ",
             sigma1 * 100, sigma2 * 100, sigma3 * 100);
    gtk_label_set_text(GTK_LABEL(widgets->phasor_label), phasor_text);

    // Update plots
    PlotWidget *signal_plot = PLOT_WIDGET(widgets->signal_plot);
    PlotWidget *time_plot = PLOT_WIDGET(widgets->time_plot);
    PlotWidget *phasor_plot = PLOT_WIDGET(widgets->phasor_plot);
    plot_widget_set_data(signal_plot, signal, noisy_signal, PLOT_TYPE_SIGNAL, seed);
    plot_widget_set_data(time_plot, signal, noisy_signal, PLOT_TYPE_TIME, seed);
    plot_widget_set_data(phasor_plot, signal, noisy_signal, PLOT_TYPE_PHASOR, seed);
    gtk_widget_queue_draw(widgets->signal_plot);
    gtk_widget_queue_draw(widgets->time_plot);
    gtk_widget_queue_draw(widgets->phasor_plot);
}

static void activate(GtkApplication *app, gpointer user_data) {
    AppWidgets *widgets = static_cast<AppWidgets*>(user_data);

    // Create main window
    widgets->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "AWGN Simulation");
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), 800, 600);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "window { background-color: #C0C0C0; }"
        "frame { border: 2px outset #C0C0C0; background-color: #C0C0C0; padding: 4px; }"
        "label { font-family: 'MS Sans Serif', 'Courier', monospace; font-size: 10pt; }"
        "entry { background-color: #FFFFFF; border: 2px inset #C0C0C0; font-family: 'MS Sans Serif', 'Courier', monospace; font-size: 10pt; padding: 2px; }"
        "button { background-color: #C0C0C0; border: 2px outset #C0C0C0; font-family: 'MS Sans Serif', 'Courier', monospace; font-size: 10pt; font-weight: bold; padding: 4px; }"
        "button:hover { background-color: #D0D0D0; }"
        "button:active { border: 2px inset #C0C0C0; }"
        "notebook { background-color: #C0C0C0; }"
        "notebook tab { background-color: #C0C0C0; border: 2px outset #C0C0C0; font-family: 'MS Sans Serif', 'Courier', monospace; font-size: 10pt; padding: 4px; }"
        "notebook tab:checked { background-color: #D0D0D0; border: 2px inset #C0C0C0; }");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Create main vertical box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);

    // Input frame
    GtkWidget *input_frame = gtk_frame_new("<b>Signal Parameters</b>");
    gtk_frame_set_label_align(GTK_FRAME(input_frame), 0.0);
    gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(input_frame))), TRUE);

    // Input grid
    GtkWidget *input_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(input_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(input_grid), 10);
    gtk_widget_set_margin_start(input_grid, 10);
    gtk_widget_set_margin_end(input_grid, 10);
    gtk_widget_set_margin_top(input_grid, 10);
    gtk_widget_set_margin_bottom(input_grid, 10);

    // Input fields
    GtkWidget *amplitude_label = gtk_label_new("Amplitude:");
    gtk_widget_set_halign(amplitude_label, GTK_ALIGN_END);
    widgets->amplitude_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widgets->amplitude_entry), "1.0");
    gtk_widget_set_tooltip_text(widgets->amplitude_entry, "Signal amplitude (> 0)");

    GtkWidget *frequency_label = gtk_label_new("Frequency (cycles/sample):");
    gtk_widget_set_halign(frequency_label, GTK_ALIGN_END);
    widgets->frequency_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widgets->frequency_entry), "0.05");
    gtk_widget_set_tooltip_text(widgets->frequency_entry, "Signal frequency (> 0)");

    GtkWidget *samples_label = gtk_label_new("Number of Samples:");
    gtk_widget_set_halign(samples_label, GTK_ALIGN_END);
    widgets->samples_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widgets->samples_entry), "1000");
    gtk_widget_set_tooltip_text(widgets->samples_entry, "Number of samples (1 to 100,000)");

    GtkWidget *snr_label = gtk_label_new("SNR (dB):");
    gtk_widget_set_halign(snr_label, GTK_ALIGN_END);
    widgets->snr_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widgets->snr_entry), "10.0");
    gtk_widget_set_tooltip_text(widgets->snr_entry, "Signal-to-Noise Ratio (>= 0)");

    GtkWidget *bandwidth_label = gtk_label_new("Bandwidth (cycles/sample):");
    gtk_widget_set_halign(bandwidth_label, GTK_ALIGN_END);
    widgets->bandwidth_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widgets->bandwidth_entry), "0.1");
    gtk_widget_set_tooltip_text(widgets->bandwidth_entry, "Filter bandwidth (> 0)");

    GtkWidget *seed_label = gtk_label_new("Seed:");
    gtk_widget_set_halign(seed_label, GTK_ALIGN_END);
    widgets->seed_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(widgets->seed_entry), "0");
    gtk_widget_set_tooltip_text(widgets->seed_entry, "Random seed (non-negative integer)");

    // Attach inputs to grid
    gtk_grid_attach(GTK_GRID(input_grid), amplitude_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), widgets->amplitude_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), frequency_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), widgets->frequency_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), samples_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), widgets->samples_entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), snr_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), widgets->snr_entry, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), bandwidth_label, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), widgets->bandwidth_entry, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), seed_label, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), widgets->seed_entry, 1, 5, 1, 1);

    gtk_frame_set_child(GTK_FRAME(input_frame), input_grid);

    // Button box
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    widgets->generate_button = gtk_button_new_with_label("Generate");
    widgets->reset_button = gtk_button_new_with_label("Reset");
    gtk_box_append(GTK_BOX(button_box), widgets->generate_button);
    gtk_box_append(GTK_BOX(button_box), widgets->reset_button);

    // Notebook for tabs
    widgets->notebook = gtk_notebook_new();
    gtk_widget_set_vexpand(widgets->notebook, TRUE);

    // Signal plot tab
    GtkWidget *signal_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    widgets->signal_plot = GTK_WIDGET(plot_widget_new());
    gtk_widget_set_vexpand(widgets->signal_plot, TRUE);
    gtk_box_append(GTK_BOX(signal_box), widgets->signal_plot);
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets->notebook), signal_box, gtk_label_new("Signal Plot"));

    // Time domain tab
    GtkWidget *time_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    widgets->time_plot = GTK_WIDGET(plot_widget_new());
    gtk_widget_set_vexpand(widgets->time_plot, TRUE);
    widgets->time_label = gtk_label_new("Zero Crossings: N/A");
    gtk_widget_set_margin_start(widgets->time_label, 10);
    gtk_widget_set_margin_end(widgets->time_label, 10);
    gtk_widget_set_margin_top(widgets->time_label, 10);
    gtk_box_append(GTK_BOX(time_box), widgets->time_plot);
    gtk_box_append(GTK_BOX(time_box), widgets->time_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets->notebook), time_box, gtk_label_new("Time Domain"));

    // Phasor plot tab
    GtkWidget *phasor_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    widgets->phasor_plot = GTK_WIDGET(plot_widget_new());
    gtk_widget_set_vexpand(widgets->phasor_plot, TRUE);
    widgets->phasor_label = gtk_label_new("Phasor Statistics: N/A");
    gtk_widget_set_margin_start(widgets->phasor_label, 10);
    gtk_widget_set_margin_end(widgets->phasor_label, 10);
    gtk_widget_set_margin_top(widgets->phasor_label, 10);
    gtk_box_append(GTK_BOX(phasor_box), widgets->phasor_plot);
    gtk_box_append(GTK_BOX(phasor_box), widgets->phasor_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(widgets->notebook), phasor_box, gtk_label_new("Phasor Plot"));

    // Assemble main box
    gtk_box_append(GTK_BOX(main_box), input_frame);
    gtk_box_append(GTK_BOX(main_box), button_box);
    gtk_box_append(GTK_BOX(main_box), widgets->notebook);

    // Connect signals
    g_signal_connect(widgets->generate_button, "clicked", G_CALLBACK(generate_signals), widgets);
    g_signal_connect(widgets->reset_button, "clicked", G_CALLBACK(reset_inputs), widgets);

    // Set main box as window content
    gtk_window_set_child(GTK_WINDOW(widgets->window), main_box);
    gtk_window_present(GTK_WINDOW(widgets->window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("com.example.awgn_simulation", G_APPLICATION_DEFAULT_FLAGS);
    AppWidgets widgets;

    g_signal_connect(app, "activate", G_CALLBACK(activate), &widgets);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}