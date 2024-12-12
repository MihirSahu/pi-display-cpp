#include "json.hpp"
//#include "resources.c"
#include "utils.cpp"
#include <curl/curl.h>
#include <gtkmm.h>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <giomm/resource.h>
#include <chrono>
#include <ctime>

//extern GResource *styles_get_resource();

using json = nlohmann::json;

// metric structs

struct UpdateScreenArgs {
  std::unordered_map<std::string, std::string> *data;
  std::vector<std::string> *data_order;
  Gtk::Label *label;
  int *data_index;
  Glib::RefPtr<Gtk::CssProvider> *css_provider;
  std::string *css;
};

struct UpdateTimeArgs {
  std::unordered_map<std::string, std::string> *data;
  Gtk::Label *label;
};

struct UpdateSystemMetricsArgs {
  std::unordered_map<std::string, std::string> *data;
  Gtk::Label *metrics_label;
};

struct UpdateTemperatureArgs {
  std::unordered_map<std::string, std::string> *data;
  Gtk::Label *temperature_label;
};

struct UpdateLifeArgs {
  std::unordered_map<std::string, std::string> *data;
  Gtk::Label *life_days_label;
  Gtk::Label *life_hours_label;
  Gtk::ProgressBar *progress_bar;
};

// metric update functions

gboolean update_time (gpointer args) {
  std::cout << "Updating time" << std::endl;
  UpdateTimeArgs *new_args = (UpdateTimeArgs *)args;
  std::unordered_map<std::string, std::string> *data = new_args->data;
  Gtk::Label *new_label = static_cast<Gtk::Label *>(new_args->label);

  std::time_t now = std::time(nullptr);
  std::tm* local_tm = std::localtime(&now);
  std::ostringstream oss;
  oss << std::put_time(local_tm, "%A, %B %d - %I:%M:%S %p");
  data->at("time") = oss.str();

  (*new_label).set_text(data->at("time"));

  return TRUE;
}

gboolean update_system_metrics(gpointer args) {
  std::cout << "Updating system metrics" << std::endl;
  UpdateSystemMetricsArgs *new_args = (UpdateSystemMetricsArgs *)args;
  std::unordered_map<std::string, std::string> *data = new_args->data;
  Gtk::Label *new_metrics_label = static_cast<Gtk::Label *>(new_args->metrics_label);

  std::string memory_usage = getMemoryUsage();
  std::string uptime = getUptime();
  std::string pi_temperature = getCPUTemperature();

  data->at("memory_usage") = memory_usage;
  data->at("uptime") = uptime;
  data->at("pi_temperature") = pi_temperature;
  new_metrics_label->set_text(/*"CPU - " + data->at("CPU") + " RAM - "*/ "RAM - " + data->at("memory_usage") + " Uptime - " + data->at("uptime") + " Pi Temp - " + data->at("pi_temperature"));

  return TRUE;
};

gboolean update_temperature(gpointer args) {
  std::cout << "Updating temperature" << std::endl;
  UpdateTemperatureArgs *new_args = (UpdateTemperatureArgs *)args;
  std::unordered_map<std::string, std::string> *data = new_args->data;
  Gtk::Label *new_temperature_label = static_cast<Gtk::Label *>(new_args->temperature_label);

  std::string temperature = get_temperature();
  data->at("temperature") = temperature + "\302\260F";
  new_temperature_label->set_text(data->at("temperature"));

  return TRUE;
}

gboolean update_life(gpointer args) {
  std::cout << "Updating life progress bar" << std::endl;
  UpdateLifeArgs *new_args = (UpdateLifeArgs*)args;
  std::unordered_map<std::string, std::string> *data = new_args->data;
  Gtk::Label *life_days_label = static_cast<Gtk::Label *>(new_args->life_days_label);
  Gtk::Label *life_hours_label = static_cast<Gtk::Label *>(new_args->life_hours_label);
  Gtk::ProgressBar *progress_bar = new_args->progress_bar;

  std::chrono::year_month_day birth_date = std::chrono::year{2002} / std::chrono::August / 11;
  float days_in_ninety_years = 32872.5;
  const auto now = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()));
  auto age_in_days = std::chrono::duration_cast<std::chrono::days>(std::chrono::sys_days(now) - std::chrono::sys_days(birth_date)).count();
  auto days_left = days_in_ninety_years - age_in_days;
  auto hours_left = days_left * 24;
  data->at("life") = std::to_string(age_in_days);

  std::cout << "Birth date: " << birth_date << "\n" << "Days in 90 years: " << days_in_ninety_years << "\n" << "Age in days: " << age_in_days << "\n" << "Days left: " << days_left << "\n" << "Hours left: " << hours_left << std::endl;

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << days_left;
  life_days_label->set_text(oss.str() + " days left");
  oss.str("");
  oss.clear();
  oss << std::fixed << std::setprecision(2) << hours_left;
  life_hours_label->set_text(oss.str() + " hours left");
  progress_bar->set_fraction(age_in_days / days_in_ninety_years);

  return TRUE;
}

int main(int argc, char *argv[]) {
  curl_global_init(CURL_GLOBAL_DEFAULT);

  auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

  std::unordered_map<std::string, std::string> data;
  std::vector<std::string> data_order = {"cat_fact", "temperature"};
  int data_index = 0;
  std::string css = R"(
      * {
        background-color: #27292D;
      }

      .time_label {
       font-size: 400%;
        font-weight: 500;
        color: white;
        margin-top: 15px;
      }

      .temp_label {
       font-size: 400%;
        font-weight: 500;
        color: white;
        margin-top: 15px;
      }

      .time_left {
        font-size: 400%;
        font-weight: 500;
        color: white;
      }

      .metrics_label {
        font-size: 300%;
        font-weight: 500;
        color: white;
        margin-bottom: 15px;
      }

      .button1 {
        font-size: large;
        font-weight: 100;
        color: white;
        padding: 10px;
      } 
    )";

  std::time_t now = std::time(nullptr);
  std::tm* local_tm = std::localtime(&now);
  std::ostringstream oss;
  oss << std::put_time(local_tm, "%a %d, %Y");

  data["time"] = oss.str();
  data["time_timeout_id"] = "";
  data["temperature"] = "Temperature";
  data["cat_fact"] = "Cat Fact";
  //data["cpu_usage"] = "";
  data["memory_usage"] = "";
  data["uptime"] = "";
  data["pi_temperature"] = "";
  data["life"] = "2002-08-11";

  Gtk::Box main_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  main_box.set_halign(Gtk::ALIGN_FILL);
  main_box.set_valign(Gtk::ALIGN_FILL);
  main_box.set_hexpand(true);
  main_box.set_vexpand(true);
  main_box.get_style_context()->add_class("main_box");

  Gtk::Box time_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  time_box.set_halign(Gtk::ALIGN_CENTER);
  time_box.set_valign(Gtk::ALIGN_START);
  Gtk::Label time_label = Gtk::Label(data["time"]);
  time_label.get_style_context()->add_class("time_label");
  time_box.add(time_label);

  Gtk::Box temperature_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  temperature_box.set_halign(Gtk::ALIGN_FILL); // Allow it to fill horizontally
  temperature_box.set_valign(Gtk::ALIGN_START); // Align it at the top
  temperature_box.set_margin_top(20); // Add the margin
  Gtk::Label temperature_label = Gtk::Label(data["temperature"]);
  temperature_label.get_style_context()->add_class("temp_label");
  temperature_label.set_line_wrap(true);  // Enable line wrapping
  temperature_label.set_line_wrap_mode(Pango::WRAP_WORD);  // Wrap at word boundaries (or use Pango::WRAP_CHAR for character boundaries)
  temperature_label.set_max_width_chars(30);  // Set the maximum number of characters before wrapping
  temperature_box.add(temperature_label);

  Gtk::ProgressBar progress_bar;
  progress_bar.set_fraction(0.0);
  progress_bar.set_text("Loading...");
  progress_bar.set_show_text(true);
  progress_bar.get_style_context()->add_class("progressbar");
  progress_bar.set_size_request(100, 10);

  Gtk::Box life_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  life_box.set_halign(Gtk::ALIGN_CENTER);
  life_box.set_valign(Gtk::ALIGN_CENTER);
  Gtk::Label life_days_label = Gtk::Label(data["life"]);
  life_days_label.get_style_context()->add_class("time_left");
  life_days_label.set_line_wrap(true);  // Enable line wrapping
  life_days_label.set_line_wrap_mode(Pango::WRAP_WORD);  // Wrap at word boundaries (or use Pango::WRAP_CHAR for character boundaries)
  life_days_label.set_max_width_chars(30);  // Set the maximum number of characters before wrapping
  Gtk::Label life_hours_label = Gtk::Label(data["life"]);
  life_hours_label.get_style_context()->add_class("time_left");
  life_hours_label.set_line_wrap(true);  // Enable line wrapping
  life_hours_label.set_line_wrap_mode(Pango::WRAP_WORD);  // Wrap at word boundaries (or use Pango::WRAP_CHAR for character boundaries)
  life_hours_label.set_max_width_chars(30);  // Set the maximum number of characters before wrapping
  life_box.add(life_days_label);
  life_box.add(life_hours_label);
  life_box.add(progress_bar);

  Gtk::Box metrics_box = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  metrics_box.set_halign(Gtk::ALIGN_CENTER);
  metrics_box.set_valign(Gtk::ALIGN_END);
  Gtk::Label metrics_label = Gtk::Label(/*"CPU - " + data["CPU"] + " RAM - "*/ "RAM - " + data["memory_usage"] + " Uptime - " + data["uptime"] + " Pi Temp - " + data["pi_temperature"]);
  metrics_label.get_style_context()->add_class("metrics_label");
  metrics_box.add(metrics_label);

  main_box.pack_start(time_box);
  main_box.pack_start(life_box);
  main_box.pack_start(temperature_box);
  main_box.pack_end(metrics_box);

  //--overlay-light: #383B40;
  //--overlay-dark: #2D2F34;
  //--surface: #27292D;
  //--base: #1F2023;
  //--overflow: #010101;

  auto css_provider = Gtk::CssProvider::create();
  css_provider->signal_parsing_error().connect(
      [](const Glib::RefPtr<const Gtk::CssSection> &section,
         const Glib::Error &error) {
        std::cerr << "CSS parsing error: " << error.what() << std::endl;
      });
  css_provider->load_from_data(css);
  Gtk::StyleContext::add_provider_for_screen(
      Gdk::Screen::get_default(), css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  UpdateTimeArgs timeArgs = {&data, &time_label};
  UpdateSystemMetricsArgs systemMetricsArgs = {&data, &metrics_label};
  UpdateTemperatureArgs temperatureArgs = {&data, &temperature_label};
  UpdateLifeArgs lifeArgs = {&data, &life_days_label, &life_hours_label, &progress_bar};

  // Update everything one time on startup
  update_system_metrics(&systemMetricsArgs);
  update_time(&timeArgs);
  update_temperature(&temperatureArgs);
  update_life(&lifeArgs);

  // Start recurring functions
  guint update_system_metrics_timeout_id = g_timeout_add_seconds(60, &update_system_metrics, &systemMetricsArgs);
  guint update_time_timeout_id = g_timeout_add_seconds(1, &update_time, &timeArgs);
  guint update_temperature_timeout_id = g_timeout_add_seconds(1800, &update_temperature, &temperatureArgs);
  guint update_life_timeout_id = g_timeout_add_seconds(3600, &update_life, &lifeArgs);

  Gtk::Window window;
  window.set_default_size(400, 300);
  window.set_title("Pi Dashboard");
  window.add(main_box);
  main_box.show_all();
  life_box.show_all();
  metrics_box.show_all();

  window.fullscreen();

  return app->run(window);
}
