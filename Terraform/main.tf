provider "google" {
  project = "fehura"
  region  = "us-central1"
  zone    = "us-central1-c"
}


resource "google_compute_instance" "vm_instance" {
  name         = "weatherstation"
  machine_type = "e2-micro"

  boot_disk {
    initialize_params {
      image = "debian-cloud/debian-12"
      size  = 30
      type  = "pd-standard"
    }
  }

  network_interface {
    network = "default"
    access_config {
    }
  }
  metadata = {
    ssh-keys = "brian:ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIFaH1Vx6DS3ILuVco40TrsFA1SKAcVJS+GZjksKEXfHk"
  }
}

# Obtain Server IP for ansible
output "server_ip" {
  value = google_compute_instance.vm_instance.network_interface.0.access_config.0.nat_ip
}

# Create Ansible inventory.ini for running
resource "local_file" "deploy_inventory" {
  content = <<EOF
[servers]
weatherstation ansible_host=${google_compute_instance.vm_instance.network_interface.0.access_config.0.nat_ip} ansible_user=brian
EOF

  filename = "../Ansible/inventory.ini"
}