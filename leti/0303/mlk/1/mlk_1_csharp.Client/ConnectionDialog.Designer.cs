namespace DevoidTalk.Client
{
    partial class ConnectionDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.TableLayoutPanel tableLayoutPanel;
            System.Windows.Forms.Label labelServerAddress;
            System.Windows.Forms.FlowLayoutPanel flowLayoutPanel;
            System.Windows.Forms.Label labelChatName;
            this.buttonConnect = new System.Windows.Forms.Button();
            this.buttonExit = new System.Windows.Forms.Button();
            this.textBoxServerAddress = new System.Windows.Forms.TextBox();
            this.textBoxChatName = new System.Windows.Forms.TextBox();
            tableLayoutPanel = new System.Windows.Forms.TableLayoutPanel();
            labelServerAddress = new System.Windows.Forms.Label();
            flowLayoutPanel = new System.Windows.Forms.FlowLayoutPanel();
            labelChatName = new System.Windows.Forms.Label();
            tableLayoutPanel.SuspendLayout();
            flowLayoutPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // tableLayoutPanel
            // 
            tableLayoutPanel.ColumnCount = 2;
            tableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            tableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            tableLayoutPanel.Controls.Add(labelServerAddress, 0, 0);
            tableLayoutPanel.Controls.Add(flowLayoutPanel, 0, 2);
            tableLayoutPanel.Controls.Add(this.textBoxServerAddress, 1, 0);
            tableLayoutPanel.Controls.Add(labelChatName, 0, 1);
            tableLayoutPanel.Controls.Add(this.textBoxChatName, 1, 1);
            tableLayoutPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            tableLayoutPanel.Location = new System.Drawing.Point(5, 5);
            tableLayoutPanel.Margin = new System.Windows.Forms.Padding(0);
            tableLayoutPanel.Name = "tableLayoutPanel";
            tableLayoutPanel.RowCount = 3;
            tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle());
            tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            tableLayoutPanel.Size = new System.Drawing.Size(309, 102);
            tableLayoutPanel.TabIndex = 0;
            // 
            // labelServerAddress
            // 
            labelServerAddress.AutoSize = true;
            labelServerAddress.Dock = System.Windows.Forms.DockStyle.Fill;
            labelServerAddress.Location = new System.Drawing.Point(3, 0);
            labelServerAddress.Name = "labelServerAddress";
            labelServerAddress.Size = new System.Drawing.Size(96, 31);
            labelServerAddress.TabIndex = 0;
            labelServerAddress.Text = "Server Address";
            labelServerAddress.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // flowLayoutPanel
            // 
            flowLayoutPanel.AutoSize = true;
            flowLayoutPanel.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            tableLayoutPanel.SetColumnSpan(flowLayoutPanel, 2);
            flowLayoutPanel.Controls.Add(this.buttonConnect);
            flowLayoutPanel.Controls.Add(this.buttonExit);
            flowLayoutPanel.Dock = System.Windows.Forms.DockStyle.Right;
            flowLayoutPanel.Location = new System.Drawing.Point(175, 62);
            flowLayoutPanel.Margin = new System.Windows.Forms.Padding(0);
            flowLayoutPanel.Name = "flowLayoutPanel";
            flowLayoutPanel.Size = new System.Drawing.Size(134, 40);
            flowLayoutPanel.TabIndex = 1;
            // 
            // buttonConnect
            // 
            this.buttonConnect.AutoSize = true;
            this.buttonConnect.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.buttonConnect.Location = new System.Drawing.Point(3, 3);
            this.buttonConnect.Name = "buttonConnect";
            this.buttonConnect.Padding = new System.Windows.Forms.Padding(5);
            this.buttonConnect.Size = new System.Drawing.Size(74, 33);
            this.buttonConnect.TabIndex = 1;
            this.buttonConnect.Text = "Connect";
            this.buttonConnect.UseVisualStyleBackColor = true;
            this.buttonConnect.Click += new System.EventHandler(this.buttonConnect_Click);
            // 
            // buttonExit
            // 
            this.buttonExit.AutoSize = true;
            this.buttonExit.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.buttonExit.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonExit.Location = new System.Drawing.Point(83, 3);
            this.buttonExit.Name = "buttonExit";
            this.buttonExit.Padding = new System.Windows.Forms.Padding(5);
            this.buttonExit.Size = new System.Drawing.Size(48, 33);
            this.buttonExit.TabIndex = 2;
            this.buttonExit.Text = "Exit";
            this.buttonExit.UseVisualStyleBackColor = true;
            this.buttonExit.Click += new System.EventHandler(this.buttonExit_Click);
            // 
            // textBoxServerAddress
            // 
            this.textBoxServerAddress.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxServerAddress.Font = new System.Drawing.Font("Verdana", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.textBoxServerAddress.Location = new System.Drawing.Point(105, 3);
            this.textBoxServerAddress.Name = "textBoxServerAddress";
            this.textBoxServerAddress.Size = new System.Drawing.Size(201, 26);
            this.textBoxServerAddress.TabIndex = 2;
            // 
            // labelChatName
            // 
            labelChatName.AutoSize = true;
            labelChatName.Dock = System.Windows.Forms.DockStyle.Fill;
            labelChatName.Location = new System.Drawing.Point(3, 31);
            labelChatName.Name = "labelChatName";
            labelChatName.Size = new System.Drawing.Size(96, 31);
            labelChatName.TabIndex = 3;
            labelChatName.Text = "Name in chat";
            labelChatName.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // textBoxChatName
            // 
            this.textBoxChatName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxChatName.Font = new System.Drawing.Font("Verdana", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.textBoxChatName.Location = new System.Drawing.Point(105, 34);
            this.textBoxChatName.Name = "textBoxChatName";
            this.textBoxChatName.Size = new System.Drawing.Size(201, 26);
            this.textBoxChatName.TabIndex = 4;
            // 
            // ConnectionDialog
            // 
            this.AcceptButton = this.buttonConnect;
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonExit;
            this.ClientSize = new System.Drawing.Size(319, 112);
            this.Controls.Add(tableLayoutPanel);
            this.Font = new System.Drawing.Font("Verdana", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ConnectionDialog";
            this.Padding = new System.Windows.Forms.Padding(5);
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Connect...";
            tableLayoutPanel.ResumeLayout(false);
            tableLayoutPanel.PerformLayout();
            flowLayoutPanel.ResumeLayout(false);
            flowLayoutPanel.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button buttonConnect;
        private System.Windows.Forms.Button buttonExit;
        private System.Windows.Forms.TextBox textBoxServerAddress;
        private System.Windows.Forms.TextBox textBoxChatName;
    }
}